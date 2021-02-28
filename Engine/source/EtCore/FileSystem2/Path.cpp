#include "stdafx.h"
#include "Path.h"


namespace et {
namespace core {


//------------------------
// Path::Component::c-tor
//
Path::Component::Component(uint16 const start, uint16 const length, std::string const& inPath)
	: m_Start(start)
	, m_Length(length)
	, m_Hash(inPath.substr(start, length).c_str())
{ }


//======
// Path
//======


// static
//////////
std::string const Path::s_InvalidString;

#ifdef ET_PLATFORM_WIN
std::vector<char> const Path::s_Delimiters{ '/', '\\' };
#else 
std::vector<char> const Path::s_Delimiters{ '/' };
#endif


//---------------
// Path::IsDelim
//
bool Path::IsDelim(char const c)
{
	return (std::find(s_Delimiters.cbegin(), s_Delimiters.cend(), c) != s_Delimiters.cend());
}

// construct destruct
//////////////////////

//-------------
// Path::c-tor
//
// Create path from string
//
Path::Path(std::string const& inPath)
{
	if (inPath.empty())
	{
		return;
	}

	size_t readPos = 0u;
	m_Impl.reserve(inPath.size());

	// read the volume, path start, path type
	//----------------------------------------
#ifdef ET_PLATFORM_WIN
	size_t volumeDelim = inPath.find(':');
	if (volumeDelim != std::string::npos)
	{
		m_Type = E_Type::AbsoluteVolume;
		volumeDelim++;
		ET_ASSERT(IsDelim(inPath[volumeDelim]), "Relative paths specifiying volumes are not supported");

		m_Impl += inPath.substr(0u, volumeDelim) + s_Delimiters[0];
		volumeDelim++;

		m_Components.emplace_back(0u, static_cast<uint16>(volumeDelim), m_Impl);

		readPos = volumeDelim;
	}
	else 
#endif
	if (IsDelim(inPath[readPos]))
	{
		m_Type = E_Type::Absolute;

		m_Impl += s_Delimiters[0];
		m_Components.emplace_back(static_cast<uint16>(readPos++), 1u, m_Impl);
	}
#ifdef ET_PLATFORM_LINUX
	else if (inPath(readPos) == '~')
	{
		m_Type = E_Type::Absolute;

		m_Impl += '~';
		m_Components.emplace_back(static_cast<uint16>(readPos++), 1u, m_Impl);
	}
#endif
	else
	{
		m_Type = E_Type::Relative;
	}

	// read components
	//-----------------
	size_t lastStart = readPos;
	while (readPos < inPath.size())
	{
		if (IsDelim(inPath[readPos]))
		{
			if (readPos == lastStart)
			{
				// we only need one component if there are multiple delimeters
				++readPos;
				lastStart = readPos;
				continue;
			}

			size_t const length = readPos - lastStart;
			size_t const writePos = m_Impl.size();
			std::string const addition = inPath.substr(lastStart, length);
			if (addition == ".")
			{
				++readPos;
				lastStart = readPos;
				continue;
			}
			else if ((addition == ".."))
			{
				if (m_Components.size() > 1u)
				{
					Component const& prev = m_Components[m_Components.size() - 2u];
					m_Impl = m_Impl.substr(0, prev.m_Start + prev.m_Length);
					m_Components.pop_back();
					++readPos;
					lastStart = readPos;
					continue;
				}

				ET_ASSERT(m_Type == E_Type::Relative, "Can't go up a directory on first absolute path component");

				if (m_Components.size() == 1u)
				{
					m_Impl.clear();
					m_Components.clear();
					++readPos;
					lastStart = readPos;
					continue;
				}
			}

			m_Impl += addition + s_Delimiters[0];
			m_Components.emplace_back(static_cast<uint16>(writePos), static_cast<uint16>(length + 1u), m_Impl);

			lastStart = readPos + 1u;
		}

		++readPos;
	}

	// read name and extension
	//-------------------------
	if (!m_Impl.empty() && IsDelim(inPath[readPos - 1u]))
	{
		m_Id.Set(m_Impl.c_str());
		return;
	}

	// either way the last part of the implementation and ID will be the rest of the input string
	size_t writePos = m_Impl.size();
	std::string const addition = inPath.substr(lastStart);
	if (addition == ".")
	{
		if (m_Components.empty())
		{
			m_Impl = "./";
			m_Components.emplace_back(0u, 2u, m_Impl);
		}

		m_Id.Set(m_Impl.c_str());
		return;
	}
	else if ((addition == ".."))
	{
		if (m_Components.size() > 1u)
		{
			Component const& prev = m_Components[m_Components.size() - 2u];
			m_Impl = m_Impl.substr(0, prev.m_Start + prev.m_Length);
			m_Components.pop_back();

			m_Id.Set(m_Impl.c_str());
			return;
		}

		ET_ASSERT(m_Type == E_Type::Relative, "Can't go up a directory on first absolute path component");

		if (m_Components.size() == 1u)
		{
			m_Impl = "./";
			m_Components[0] = Component(0u, 2u, m_Impl);

			m_Id.Set(m_Impl.c_str());
			return;
		}

		m_Impl = "../";
		m_Components[0] = Component(0u, 3u, m_Impl);

		m_Id.Set(m_Impl.c_str());
		return;
	}

	m_Impl += addition;
	m_Id.Set(m_Impl.c_str());

	// might have to split components however
	while (readPos > lastStart)
	{
		readPos--;
		if (inPath[readPos] == '.')
		{
			// extension found - read file name in
			size_t const length = readPos - lastStart;
			m_Components.emplace_back(static_cast<uint16>(writePos), static_cast<uint16>(length), m_Impl);

			writePos += length + 1u;
			m_HasExtension = true;

			readPos--;
			break;
		}
	}

	m_Components.emplace_back(static_cast<uint16>(writePos), static_cast<uint16>(inPath.size() - readPos), m_Impl);
}

// accessors
/////////////

//---------------
// Path::GetName
//
// Combined name + extension or directory name
//
std::string Path::GetName() const
{
	if (m_HasExtension)
	{
		ET_ASSERT(m_Components.size() > 1u);
		Component const& fileComp = m_Components[m_Components.size() - 2u];

		return m_Impl.substr(fileComp.m_Start);
	}

	if (m_Impl.empty())
	{
		return s_InvalidString;
	}

	ET_ASSERT(!m_Components.empty());
	Component const& comp = m_Components[m_Components.size() - 1u];
	return m_Impl.substr(comp.m_Start);
}

//------------------
// Path::GetRawName
//
// Name without extension
//
std::string Path::GetRawName() const
{
	if (m_HasExtension)
	{
		ET_ASSERT(m_Components.size() > 1u);
		Component const& fileComp = m_Components[m_Components.size() - 2u];

		return m_Impl.substr(fileComp.m_Start, fileComp.m_Length);
	}

	if (m_Impl.empty())
	{
		return s_InvalidString;
	}

	ET_ASSERT(!m_Components.empty());
	return m_Impl.substr(m_Components[m_Components.size() - 1u].m_Start);
}

//--------------------
// Path::GetRawNameId
//
HashString Path::GetRawNameId() const
{
	if (m_HasExtension)
	{
		ET_ASSERT(m_Components.size() > 1u);
		return m_Components[m_Components.size() - 2u].m_Hash;
	}

	if (m_Components.empty())
	{
		return HashString();
	}

	return m_Components[m_Components.size() - 1u].m_Hash;
}

//--------------------
// Path::GetExtension
//
std::string Path::GetExtension() const
{
	if (m_HasExtension)
	{
		ET_ASSERT(!m_Components.empty());
		return m_Impl.substr(m_Components[m_Components.size() - 1u].m_Start);
	}

	return s_InvalidString;
}

//----------------------
// Path::GetExtensionId
//
HashString Path::GetExtensionId() const
{
	if (m_HasExtension)
	{
		ET_ASSERT(!m_Components.empty());
		return m_Components[m_Components.size() - 1u].m_Hash;
	}

	return HashString();
}

//---------------------
// Path::GetParentPath
//
std::string Path::GetParentPath() const
{
	size_t const size = m_Components.size();
	size_t const offset = m_HasExtension ? 3u : 2u;
	if (size >= offset)
	{
		Component const& comp = m_Components[size - offset];
		return m_Impl.substr(0u, comp.m_Start + comp.m_Length);
	}

	return s_InvalidString;
}

//---------------------
// Path::HasParentPath
//
bool Path::HasParentPath() const
{
	return m_Components.size() >= (m_HasExtension ? 3u : 2u);
}

//--------------
// Path::IsFile
//
bool Path::IsFile() const
{
	if (m_Impl.empty())
	{
		return false;
	}
	
	return (m_Impl[m_Impl.size() - 1u] != s_Delimiters[0u]);
}

//-----------------
// Path::GetVolume
//
std::string Path::GetVolume() const
{
	if (!m_Impl.empty() && (m_Type != E_Type::Relative))
	{
		return m_Impl.substr(0u, m_Components[0u].m_Start + m_Components[0u].m_Length);
	}

	return s_InvalidString;
}

//-------------------
// Path::GetVolumeId
//
HashString Path::GetVolumeId() const
{
	if (!m_Impl.empty() && (m_Type != E_Type::Relative))
	{
		return m_Components[0].m_Hash;
	}

	return HashString();
}

//--------------
// Path::IsThis
//
bool Path::IsThis(Component const& comp)
{
	return ((comp.m_Hash == "./"_hash) || (comp.m_Hash == "."_hash));
}

//----------------
// Path::IsParent
//
bool Path::IsParent(Component const& comp)
{
	return ((comp.m_Hash == "../"_hash) || (comp.m_Hash == ".."_hash));
}


} // namespace core
} // namespace et
