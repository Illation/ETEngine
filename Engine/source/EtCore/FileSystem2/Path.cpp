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
			m_Impl += inPath.substr(lastStart, length) + s_Delimiters[0];
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
	m_Impl += inPath.substr(lastStart);
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
et::core::HashString Path::GetRawNameId() const
{
	if (m_HasExtension)
	{
		ET_ASSERT(m_Components.size() > 1u);
		return m_Components[m_Components.size() - 2u].m_Hash;
	}

	if (m_Components.empty())
	{
		return core::HashString();
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
et::core::HashString Path::GetExtensionId() const
{
	if (m_HasExtension)
	{
		ET_ASSERT(!m_Components.empty());
		return m_Components[m_Components.size() - 1u].m_Hash;
	}

	return core::HashString();
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
et::core::HashString Path::GetVolumeId() const
{
	if (!m_Impl.empty() && (m_Type != E_Type::Relative))
	{
		return m_Components[0].m_Hash;
	}

	return core::HashString();
}


} // namespace core
} // namespace et
