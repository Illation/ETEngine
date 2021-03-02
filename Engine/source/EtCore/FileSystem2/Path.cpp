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


// construct destruct
//////////////////////

//-------------
// Path::c-tor
//
// Create path from string
//
Path::Path(std::string const& inPath)
{
	if (inPath.empty() || (inPath == "./"))
	{
		m_Impl = "./";
		m_Components.emplace_back(0u, 2u, m_Impl);
		m_Id.Set(m_Impl.c_str());
		m_Type = E_Type::Relative;
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

	// read the rest
	//---------------
	ConstructComponentsAndName(readPos, inPath);
}

//-------------
// Path::c-tor
//
// Create path from string and a base path
//
Path::Path(Path const& base, std::string const& inPath)
{
	ET_ASSERT(!base.IsFile(), "can't use file paths as base for a new path");
	ET_ASSERT(!IsDelim(inPath[0]), "can't append absolute paths to a base path");
#ifdef ET_PLATFORM_WIN
	ET_ASSERT(inPath.find(':') == std::string::npos, "Volume paths are not supported when constructing from a base path");
#endif

	if (base.GetId() == "./"_hash)
	{
		m_Impl.reserve(inPath.size());
	}
	else
	{
		m_Impl.reserve(base.m_Impl.size() + inPath.size());
		m_Impl = base.m_Impl;
		m_Components = base.m_Components;
	}

	m_Type = base.m_Type;

	ConstructComponentsAndName(0u, inPath);
}

//-------------
// Path::c-tor
//
// Create path from string and a base path
//
Path::Path(Path const& base, Path const& other)
{
	ET_ASSERT(!base.IsFile(), "can't use file paths as base for a new path");
	ET_ASSERT(other.GetType() == E_Type::Relative, "can't append absolute paths to a base path");

	if (base.GetId() == "./"_hash)
	{
		m_Impl = other.m_Impl;
		m_Id = other.m_Id;
		m_Components = other.m_Components;
		m_Type = E_Type::Relative;
		m_HasExtension = other.m_HasExtension;
		return;
	}
	else if (other.GetId() == "./"_hash)
	{
		m_Impl = base.m_Impl;
		m_Id = base.m_Id;
		m_Components = base.m_Components;
		m_Type = base.m_Type;
		return;
	}

	size_t const baseRemoveCount = base.GetParentMoveComponentCount();
	size_t const otherRemoveCount = other.GetParentMoveComponentCount();

	m_HasExtension = other.m_HasExtension;

	size_t const baseCompCount = base.m_Components.size() - baseRemoveCount;
	size_t const otherCompCount = other.m_Components.size() - otherRemoveCount;
	if (otherRemoveCount >= baseCompCount)
	{
		size_t const parentMoves = baseRemoveCount + otherRemoveCount - baseCompCount;
		size_t const moveLen = parentMoves * 3u;

		if (otherCompCount == 0u)
		{
			m_Impl.reserve(moveLen);
			m_Components.reserve(parentMoves);

			for (size_t i = 0u; i < parentMoves * 3u; i += 3)
			{
				m_Impl += "../";
				m_Components.emplace_back(static_cast<uint16>(i), 3u, m_Impl);
			}
		}
		else
		{
			size_t const otherStart = other.m_Components[otherRemoveCount].m_Start;
			Component const& lastComp = other.m_Components[other.m_Components.size() - 1u];
			size_t const otherLen = (lastComp.m_Start + lastComp.m_Length) - otherStart;

			m_Impl.reserve(moveLen + otherLen);
			m_Components.reserve(parentMoves + otherCompCount);

			for (size_t i = 0u; i < parentMoves * 3u; i += 3)
			{
				m_Impl += "../";
				m_Components.emplace_back(static_cast<uint16>(i), 3u, m_Impl);
			}

			size_t start = m_Impl.size();
			m_Impl += other.m_Impl.substr(otherStart, otherLen);
			for (size_t i = otherRemoveCount; i < other.m_Components.size(); ++i)
			{
				if (m_HasExtension && (i == (other.m_Components.size() - 1u)))
				{
					m_Components.emplace_back(static_cast<uint16>(start + 1u), other.m_Components[i].m_Length - 1u, m_Impl);
				}
				else
				{
					m_Components.emplace_back(static_cast<uint16>(start), other.m_Components[i].m_Length, m_Impl);
				}
				
				start += other.m_Components[i].m_Length;
			}
		}
	}
	else
	{
		size_t const baseAdd = base.m_Components.size() - otherRemoveCount;
		Component const& lastBase = base.m_Components[baseAdd - 1u];
		size_t const baseLen = lastBase.m_Start + lastBase.m_Length;

		if (otherCompCount == 0u)
		{
			m_Components.reserve(baseAdd);

			m_Impl = base.m_Impl.substr(0, baseLen);
			m_Components.insert(m_Components.end(), base.m_Components.begin(), base.m_Components.begin() + baseAdd);
		}
		else
		{
			size_t const otherStart = other.m_Components[otherRemoveCount].m_Start;
			Component const& lastComp = other.m_Components[other.m_Components.size() - 1u];
			size_t const otherLen = (lastComp.m_Start + lastComp.m_Length) - otherStart;

			m_Impl.reserve(baseLen + otherLen);
			m_Components.reserve(baseAdd + otherCompCount);

			m_Impl = base.m_Impl.substr(0, baseLen);
			m_Components.insert(m_Components.end(), base.m_Components.begin(), base.m_Components.begin() + baseAdd);

			size_t start = m_Impl.size();
			m_Impl += other.m_Impl.substr(otherStart, otherLen);
			for (size_t i = otherRemoveCount; i < other.m_Components.size(); ++i)
			{
				if (m_HasExtension && (i == (other.m_Components.size() - 1u)))
				{
					m_Components.emplace_back(static_cast<uint16>(start + 1u), other.m_Components[i].m_Length - 1u, m_Impl);
				}
				else
				{
					m_Components.emplace_back(static_cast<uint16>(start), other.m_Components[i].m_Length, m_Impl);
				}

				start += other.m_Components[i].m_Length;
			}
		}
	}

	m_Id.Set(m_Impl.c_str());
	m_Type = base.m_Type;
}


// functionality
/////////////////

//--------------
// Path::Append
//
void Path::Append(Path const& other)
{
	Path temp(*this, other);
	Swap(temp);
}

//----------------
// Path::PopFront
//
void Path::PopFront()
{
	if (m_Id == "./"_hash)
	{
		return;
	}

	size_t const toRemove = (m_HasExtension && (m_Components.size() == 2u)) ? 2u : 1u;
	ET_ASSERT(m_Components.size() >= toRemove);

	m_Components.erase(m_Components.begin(), m_Components.begin() + toRemove);

	if (m_Components.empty())
	{
		m_Impl = "./";
		m_Components.emplace_back(0u, 2u, m_Impl);
		m_HasExtension = false;
	}
	else
	{
		m_Impl = m_Impl.substr(m_Components[0].m_Start);
	}

	m_Id.Set(m_Impl.c_str());

	if (m_Type != E_Type::Relative)
	{
		m_Type = E_Type::Relative;
	}
}

//---------------
// Path::PopBack
//
void Path::PopBack()
{
	if (m_Id == "./"_hash)
	{
		return;
	}

	size_t toRemove;
	if (m_HasExtension)
	{
		toRemove = 2u;
		m_HasExtension = false;
	}
	else
	{
		toRemove = 1u;
	}

	ET_ASSERT(m_Components.size() >= toRemove);

	m_Components.pop_back();
	if (toRemove > 1u)
	{
		m_Components.pop_back();
	}

	if (m_Components.empty())
	{
		m_Impl = "./";
		m_Components.emplace_back(0u, 2u, m_Impl);
		m_Type = E_Type::Relative;
	}
	else
	{
		Component const& last = m_Components[m_Components.size() - 1u];
		m_Impl = m_Impl.substr(0u, last.m_Start + last.m_Length);
	}

	m_Id.Set(m_Impl.c_str());
}

//--------------------
// Path::MakeRelative
//
void Path::MakeRelative(Path const& relativeTo)
{
	if (relativeTo.GetId() == "./"_hash)
	{
		ET_ASSERT(m_Type == E_Type::Relative); // nothing is really changing in this case
		return;
	}
	else if (m_Id == "./"_hash)
	{
		ET_ASSERT(!relativeTo.IsFile());
		m_Impl = relativeTo.m_Impl;
		m_Id = relativeTo.m_Id;
		m_Components = relativeTo.m_Components;
		m_Type = relativeTo.m_Type;
		return;
	}

	ET_ASSERT(!relativeTo.IsFile());

	std::string tempImpl;
	std::vector<Component> tempComps;

	// find first uncommon component
	size_t const minSize = std::min(m_Components.size(), relativeTo.m_Components.size());
	size_t firstDif = 0u;
	for (; firstDif < minSize; ++firstDif)
	{
		if (m_Components[firstDif].m_Hash != relativeTo.m_Components[firstDif].m_Hash)
		{
			break;
		}
	}

	// move up some directories
	for (size_t i = firstDif; i < relativeTo.m_Components.size(); ++i)
	{
		uint16 const start = static_cast<uint16>(tempImpl.length());
		tempImpl += "../";
		tempComps.emplace_back(start, 3u, tempImpl);
	}

	// add the missing directories from source
	if (firstDif < m_Components.size())
	{
		uint16 start = static_cast<uint16>(tempImpl.length());
		Component const& last = m_Components[m_Components.size() - 1u];
		tempImpl += m_Impl.substr(m_Components[firstDif].m_Start, last.m_Start + last.m_Length);
		for (size_t i = firstDif; i < m_Components.size(); ++i)
		{
			if (m_HasExtension && (i == (m_Components.size() - 1u)))
			{
				tempComps.emplace_back(start + 1u, m_Components[i].m_Length - 1u, tempImpl);
			}
			else
			{
				tempComps.emplace_back(start, m_Components[i].m_Length, tempImpl);
			}

			start += m_Components[i].m_Length;
		}
	}

	// resolve
	m_Impl.swap(tempImpl);
	m_Id.Set(m_Impl.c_str());
	m_Components.swap(tempComps);
	m_Type = E_Type::Relative; // extension status will stay the same
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
	if (m_Impl.size() <= 1u) // size 1 delim still means it's an absolute directory so it's not a file
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


// utility
///////////

//------------------
// Path::Swap
//
void Path::Swap(Path& other)
{
	std::swap(m_Impl, other.m_Impl);
	std::swap(m_Id, other.m_Id);
	std::swap(m_Components, other.m_Components);
	std::swap(m_Type, other.m_Type);
	std::swap(m_HasExtension, other.m_HasExtension);
}

//----------------------------------
// Path::ConstructComponentsAndName
//
// Common construction
//
void Path::ConstructComponentsAndName(size_t const startPos, std::string const& inPath)
{
	size_t readPos = startPos;
	size_t lastStart = readPos;

	// read components
	//-----------------
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
					if (prev.m_Hash != "../"_hash)
					{
						m_Impl = m_Impl.substr(0, prev.m_Start + prev.m_Length);
						m_Components.pop_back();
						++readPos;
						lastStart = readPos;
						continue;
					}
				}
				else if ((m_Components.size() == 1u) && (m_Components[0u].m_Hash != "../"_hash))
				{
					ET_ASSERT(m_Type == E_Type::Relative, "Can't go up a directory on first absolute path component");
					m_Impl.clear();
					m_Components.clear();
					++readPos;
					lastStart = readPos;
					continue;
				}

				ET_ASSERT(m_Type == E_Type::Relative, "Can't go up a directory on first absolute path component");
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
			if (prev.m_Hash != "../"_hash)
			{
				m_Impl = m_Impl.substr(0, prev.m_Start + prev.m_Length);
				m_Components.pop_back();

				m_Id.Set(m_Impl.c_str());
				return;
			}
		}
		else if ((m_Components.size() == 1u) && (m_Components[0u].m_Hash != "../"_hash))
		{
			ET_ASSERT(m_Type == E_Type::Relative, "Can't go up a directory on first absolute path component");
			m_Impl = "./";
			m_Components[0] = Component(0u, 2u, m_Impl);

			m_Id.Set(m_Impl.c_str());
			return;
		}

		ET_ASSERT(m_Type == E_Type::Relative, "Can't go up a directory on first absolute path component");
		m_Impl += "../";
		m_Components.emplace_back(0u, 3u, m_Impl);

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

//-------------------------------
// Path::GetParentComponentCount
//
// Number of directories to move up (../)
//
size_t Path::GetParentMoveComponentCount() const
{
	size_t count = 0u;
	for (Component const& comp : m_Components)
	{
		if (IsParent(comp))
		{
			count++;
			continue;
		}

		break;
	}

	return count;
}


} // namespace core
} // namespace et
