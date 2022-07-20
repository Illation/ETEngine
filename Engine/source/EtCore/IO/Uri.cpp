#include "stdafx.h"
#include "Uri.h"

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>


namespace et {
namespace core {


//============================
// Unique Resource Identifier
//============================


// static
std::string const URI::s_Base64Mime("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
std::string const URI::s_UriFile("file");
std::string const URI::s_UriData("data");


//-------------------
// URI::DecodeBase64
//
// Convert a Base64 string into regular bytes
//
bool URI::DecodeBase64(std::string const& encoded, std::vector<uint8>& decoded)
{
	size_t in_len = encoded.size();
	uint32 i = 0;
	uint8 char_array_4[4], char_array_3[3];

	while (in_len-- && (encoded[i] != '='))
	{
		if (!IsBase64(encoded[i]))
		{
			return false;
		}

		char_array_4[i % 4] = static_cast<uint8>(s_Base64Mime.find(encoded[i]));
		if (++i % 4 == 0)
		{
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (uint32 j = 0; j < 3; j++)
			{
				decoded.push_back(char_array_3[j]);
			}
		}
	}

	if (i % 4)
	{
		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

		for (uint32 j = 0; (j < i % 4 - 1); j++)
		{
			decoded.push_back(char_array_3[j]);
		}
	}

	return true;
}

//------------
// URI::c-tor
//
// Contruct from path
//
URI::URI(std::string const& path)
{
	SetPath(path);
}

//--------------
// URI::SetPath
//
// We find out what type of URI this is here
//
void URI::SetPath(std::string const& path)
{
	if (m_IsEvaluated)
	{
		ClearEvaluatedData();
	}

	if (path.empty())
	{
		m_Path.clear();
		m_Type = URI::E_Type::None;
		return;
	}

	size_t const colonPos = path.find(':');
	if ((colonPos == std::string::npos) || (colonPos == 0u))
	{
		m_Path = path;
		m_Type = URI::E_Type::FileRelative;
		return;
	}

	std::string const typeStr = path.substr(0u, colonPos);
	if (typeStr == s_UriData)
	{
		m_Type = URI::E_Type::Data;
		m_Path = path.substr(colonPos + 1u);
	}
	else if (typeStr == s_UriFile)
	{
		if (path.substr(colonPos + 1u, 3u) != "///")
		{
			m_Type = URI::E_Type::Invalid;
			ET_ERROR("Only files on local host are currently supported!");
			return;
		}

		m_Type = URI::E_Type::FileAbsolute;
		m_Path = path.substr(colonPos + 4u);
	}
	else
	{
		m_Type = URI::E_Type::Invalid;
		ET_ERROR("Unsupported URI type");
		return;
	}

	// replace spaces
	size_t foundIdx = m_Path.length();
	while (true)
	{
		foundIdx = m_Path.rfind("%20", foundIdx);
		if (foundIdx == std::string::npos)
		{
			break;
		}

		m_Path.replace(foundIdx, 3u, 1u, ' ');
	}
}

//---------------
// URI::Evaluate
//
// Evaluate without providing a base path
//
bool URI::Evaluate()
{
	static std::string const s_EmptyBasePath;
	return Evaluate(s_EmptyBasePath);
}

//---------------
// URI::Evaluate
//
// Parse a URI string into the URI struct
//
bool URI::Evaluate(std::string const& basePath)
{
	switch (m_Type)
	{
	case E_Type::Invalid:
		ET_WARNING("Can't evaluate invalid URI");
		return false;

	case E_Type::None:
		return true;

	case E_Type::Data:
	{
		size_t const dataPos = m_Path.find(',');
		if (dataPos == std::string::npos)
		{
			ET_TRACE_W(ET_CTX_CORE, "couldn't find data uri data");
			return false;
		}

		std::string mediatype = m_Path.substr(0u, dataPos);
		size_t const paramPos = mediatype.find(';');
		if (paramPos == std::string::npos)
		{
			ET_TRACE_W(ET_CTX_CORE, "couldn't find data uri mediatype parameter");
			return false;
		}

		std::string const parameter = mediatype.substr(paramPos + 1u);
		mediatype = mediatype.substr(0, paramPos);

		size_t const subtypePos = mediatype.find('/');
		if (subtypePos == std::string::npos)
		{
			ET_TRACE_W(ET_CTX_CORE, "couldn't find data uri mediatype subtype");
			return false;
		}

		m_Extension = mediatype.substr(subtypePos + 1u);
		mediatype = mediatype.substr(0u, subtypePos);

		std::string const dataString = m_Path.substr(dataPos + 1u);
		if (parameter == "base64")
		{
			if (DecodeBase64(dataString, m_BinData))
			{
				m_Path = m_Path.substr(0u, dataPos);
				std::string(m_Path).swap(m_Path);//free that memory
				return true;
			}
		}

		return false;
	}

	case E_Type::FileRelative:
	{
		{
			core::Directory* const dir = new core::Directory(basePath, nullptr);
			core::File* const input = new core::File(m_Path, dir);
			if (!input->Open(core::FILE_ACCESS_MODE::Read))
			{
				ET_WARNING("Failed to evaluate URI from file '%s'", m_Path.c_str());
				return false;
			}

			m_BinData = input->Read();
			m_Extension = input->GetExtension();

			delete input;
			delete dir;
		}

		return true;
	}

	case E_Type::FileAbsolute:
	{
		{
			core::File importFile(m_Path, nullptr);
			if (!importFile.Open(core::FILE_ACCESS_MODE::Read))
			{
				ET_WARNING("Failed to evaluate URI from file '%s'", m_Path.c_str());
				return false;
			}

			m_BinData = importFile.Read();
			m_Extension = importFile.GetExtension();
		}

		return true;
	}

	default:
		ET_WARNING("Unsupported URI type!");
		return false;
	}
}

//-------------------------
// URI::ClearEvaluatedData
//
void URI::ClearEvaluatedData()
{
	m_IsEvaluated = false;
	m_Extension.clear();
	m_BinData.clear();
	m_BinData.reserve(0u);
}


} // namespace core
} // namespace et
