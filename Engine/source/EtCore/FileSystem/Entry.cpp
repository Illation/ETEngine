#include "stdafx.h"
#include "Entry.h"

#include <iostream>
#include <limits>

#include "FileUtil.h"

#include <EtCore/FileSystem/Facade/FileBase.h>
#include <EtCore/FileSystem/Facade/FileAccessMode.h>
#include <EtCore/FileSystem/Facade/FileAccessFlags.h>


namespace et {
namespace core {


//======================
// Entry
//======================


//---------------------------------
// Entry::c-tor
//
// Entry constructor sets up the parent and path members
//
Entry::Entry(std::string name, Directory* parent)
{
	m_Path = FileUtil::ExtractPath(name);

	if (parent == nullptr)
	{
		m_Parent = parent;
		if (!FileUtil::IsAbsolutePath(m_Path))
		{
			m_Path = FileUtil::GetAbsolutePath(m_Path);
		}

		m_Filename = FileUtil::ExtractName(name);
	}
	else
	{
		if (!m_Path.empty() && parent->IsMounted() && parent->Exists())
		{
			m_Parent = parent->CreateSubdirectory(m_Path);
			m_Path = "/";

			m_Filename = FileUtil::ExtractName(name);
		}
		else
		{
			m_Parent = parent;
			m_Filename = name;
			m_Path = "/";
		}
	}
}

//---------------------------------
// Entry::d-tor
//
// Make sure to remove the entry from the parent during destruction
//
Entry::~Entry()
{
	if (m_Parent != nullptr)
	{
		m_Parent->RemoveChild(this);
	}
}

//---------------------------------
// Entry::GetPath
//
// Get the path of an entry, recursively stepping up the parent list
//
std::string Entry::GetPath() const
{
	if (m_Parent)
	{
		return std::string(m_Parent->GetPath() + m_Parent->GetNameOnly() + m_Path);
	}
	else
	{
		return m_Path;
	}
}

//---------------------------------
// Entry::GetName
//
// Get the name of an entry including its path
//
std::string Entry::GetName() const
{
	return GetPath() + m_Filename;
}

//---------------------------------
// Entry::GetExtension
//
// Get the extension of an entry, if it has one
//
std::string Entry::GetExtension()
{
	if(GetType() == EntryType::ENTRY_FILE)
	{
		return FileUtil::ExtractExtension(m_Filename);
	}

	return std::string("");
}


//======================
// File
//======================


//---------------------------------
// File::File
//
// File cosntructor, doesn't automatically open
//
File::File(std::string name, Directory* pParent)
	: Entry(name, pParent)
	, m_IsOpen(false)
{ }

//---------------------------------
// File::~File
//
// File destructor closes the file
//
File::~File()
{
	if (m_IsOpen)
	{
		Close();
	}
}

//---------------------------------
// File::Open
//
// Open the file
//
bool File::Open(FILE_ACCESS_MODE mode, FILE_ACCESS_FLAGS flags)
{
	std::string path = GetPath()+m_Filename;
	m_Handle = FILE_BASE::Open( path.c_str(), flags, mode);
	if (m_Handle == FILE_HANDLE_INVALID) 
	{
		ET_TRACE_E(ET_CTX_CORE, "Opening File failed");
		return false;
	}

	m_IsOpen = true;
	//If we created this file new and have a parent that doesn't know about it, add to parent
	if(m_Parent)
	{
		if(!(std::find(m_Parent->m_pChildren.begin(), m_Parent->m_pChildren.end(), this ) != m_Parent->m_pChildren.end()))
		{
			m_Parent->m_pChildren.push_back( this );
		}
	}
	return true;
}

//---------------------------------
// File::Read
//
// Read from the file
//
std::vector<uint8> File::Read()
{
	ET_ASSERT(m_IsOpen);

	std::vector<uint8> content;
	if(!FILE_BASE::ReadFile(m_Handle, content, GetSize()))
	{
		ET_TRACE_E(ET_CTX_CORE, "File::Read > Reading File failed");
	}

	return content;
}

//---------------------------------
// File::Read
//
// Read from the file
//
std::vector<uint8> File::ReadChunk(uint64 const offset, uint64 const numBytes)
{
	ET_ASSERT(m_IsOpen);
	ET_ASSERT(offset + numBytes <= GetSize(),
		"Range of bytes requested exceeds file size! Offset:'%u'; Bytes:'%u'; Total:'%u'; Size:'%u'", 
		offset, numBytes, offset + numBytes, GetSize());

	std::vector<uint8> content;
	if (!FILE_BASE::ReadFile(m_Handle, content, numBytes, offset))
	{
		ET_TRACE_E(ET_CTX_CORE, "File::ReadChunk > Reading file failed");
	}

	return content;
}

//---------------------------------
// File::Write
//
// Write to the file
//
bool File::Write(const std::vector<uint8> &lhs)
{
	if ( !FILE_BASE::WriteFile(m_Handle, lhs) )
	{
		ET_TRACE_E(ET_CTX_CORE, "Writing File failed");
        return false;
    }

	return true;
}

//---------------------------------
// File::Close
//
// Close the file
//
void File::Close()
{
	if(FILE_BASE::Close( m_Handle ))
	{
		m_IsOpen = false;
	}
}

//---------------------------------
// File::GetSize
//
// Get the size of the file in bytes
//
uint64 File::GetSize()
{
	int64 size;
	if (!FILE_BASE::GetEntrySize(m_Handle, size))
	{
		ET_TRACE_E(ET_CTX_CORE, "Getting File size failed");
		return std::numeric_limits<uint64>::max();
	}

	return static_cast<uint64>(size);
}

//---------------------------------
// File::Exists
//
bool File::Exists()
{
	return FILE_BASE::Exists((GetPath() + m_Filename).c_str());
}

//---------------------------------
// File::Delete
//
// Delete the file from the disk
//
bool File::Delete()
{
	if (m_IsOpen)
	{
		Close();
	}

	if(m_IsOpen)
	{
		ET_TRACE_E(ET_CTX_CORE, "Couldn't delete file because it failed to close");
		return false;
	}

	std::string path = GetPath()+m_Filename;
	if(FILE_BASE::DeleteFile( path.c_str() ))
	{
		if(m_Parent)
		{
			m_Parent->RemoveChild( this );
		}

		delete this;
		return true;
	}

	return false;
}


//======================
// Directory
//======================


//---------------------------------
// Directory::Directory
//
// Directory constructor
//
Directory::Directory(std::string name, Directory* pParent, bool ensureExists)
	:Entry(name, pParent)
{
	if (ensureExists)
	{
		Create();
	}
}

//---------------------------------
// Directory::~Directory
//
// Directory destructor, unmounts if its mounted
//
Directory::~Directory()
{
	if (m_IsMounted)
	{
		Unmount();
	}
}

//---------------------------------
// Directory::RemoveChild
//
// Remove a child from the directory
//
void Directory::RemoveChild( Entry* child )
{
	m_pChildren.erase( std::remove( m_pChildren.begin(), m_pChildren.end(), child ) , m_pChildren.end() );
}

//---------------------------------
// Directory::CreateSubdirectory
//
// returns leaf subdirectory
//
Directory* Directory::CreateSubdirectory(std::string& path)
{
	std::string const searchName = FileUtil::SplitFirstDirectory(path);
	if (searchName.empty())
	{
		return this;
	}

	auto childIt = std::find_if(m_pChildren.begin(), m_pChildren.end(), [&searchName](Entry const* const entry)
		{
			return entry->GetNameOnly() == searchName;
		});

	if (childIt == m_pChildren.cend()) // child not found
	{
		m_pChildren.push_back(new Directory(searchName, this, true));
		childIt = std::prev(m_pChildren.end());
	}

	Entry* const childEntry = *childIt;
	if (childEntry->GetType() == Entry::EntryType::ENTRY_FILE)
	{
		ET_ASSERT(false, "subdirectory path (%s) contains a file entry (%s)", (GetName() + path).c_str(), childEntry->GetName());
		return nullptr;
	}

	return static_cast<Directory*>(childEntry)->CreateSubdirectory(path);
}

//---------------------------------
// Directory::Unmount
//
// Unmount the directory and all subdirectories
//
void Directory::Unmount()
{
	if (m_pChildren.size() > 0u)
	{
		for (size_t idx = m_pChildren.size() - 1; idx < m_pChildren.size(); --idx)
		{
			if(m_pChildren[idx]->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
			{
				if (m_pChildren[idx]->GetName() != "../" && m_pChildren[idx]->GetName() != "./")
				{
					static_cast<Directory*>(m_pChildren[idx])->Unmount();
				}
			}

			delete m_pChildren[idx];
		}
	}

    m_pChildren.clear();
    m_IsMounted = false;
}

//---------------------------------
// Directory::Delete
//
// Delete this directory, also deletes the object
//
bool Directory::Delete()
{
	if (m_pChildren.size() > 0u)
	{
		for(size_t idx = m_pChildren.size() - 1; idx < m_pChildren.size(); --idx)
		{
			if(m_pChildren[idx]->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
			{
				if(m_pChildren[idx]->GetName() != "../" && m_pChildren[idx]->GetName() != "./")
				{
					if (!(m_pChildren[idx]->Delete()))
					{
						return false;
					}
				}
			}
			else
			{
				if (!(m_pChildren[idx]->Delete()))
				{
					return false;
				}
			}
		}
	}

	m_pChildren.clear();

	if(DeleteDir())
	{
		if(m_Parent)
		{
			m_Parent->RemoveChild( this );
		}

		delete this;
		return true;
	}

	return false;
}

//---------------------------------
// Directory::GetChildrenByExt
//
// Get all children by file extension
//
std::vector<Entry*> Directory::GetChildrenByExt(std::string const& ext)
{
	std::vector<Entry*> ret;
	for (auto e : m_pChildren)
	{
		if (e->GetExtension() == ext)
		{
			ret.push_back(e);
		}
	}

	return ret;
}

//---------------------------------
// Directory::GetChildrenRecursive
//
// Get all children that are files, including in mounted subdirectories
//
void Directory::GetChildrenRecursive(std::vector<File*>& children)
{
	for (Entry* child : m_pChildren)
	{
		switch (child->GetType())
		{
		case Entry::EntryType::ENTRY_DIRECTORY:
			static_cast<Directory*>(child)->GetChildrenRecursive(children);
			break;

		case Entry::EntryType::ENTRY_FILE:
			children.emplace_back(static_cast<File*>(child));
			break;

		default:
			ET_TRACE_W(ET_CTX_CORE, "Directory::GetChildrenRecursive > Unhandled entry type: '%s'", child->GetName().c_str());
			break;
		}
	}
}

//---------------------------------
// Directory::GetMountedChild
//
// Returns nullptr, or the child Entry if it is found
//
Entry* Directory::GetMountedChild(std::string const& path) const
{
	std::string modPath = path;

	FileUtil::RemoveExcessPathDelimiters(modPath);
	FileUtil::RemoveRelativePath(modPath);

	return GetMountedChildRecursive(modPath);
}

//---------------------------------
// Directory::RecursiveMount
//
// Recursively mount all subdirectories
//
void Directory::RecursiveMount()
{
	for (auto c : m_pChildren)
	{
		if (c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
		{
			if (c->GetName() != "../" && c->GetName() != "./")
			{
				static_cast<Directory*>(c)->Mount(true);
			}
		}
	}
}

//---------------------------------
// Directory::GetMountedChildRecursive
//
Entry* Directory::GetMountedChildRecursive(std::string& path) const
{
	// split off the next search directory and figure out if this is the last one
	std::string searchName = FileUtil::SplitFirstDirectory(path);

	bool isFinalEntry = searchName.empty();
	if (isFinalEntry)
	{
		searchName = path;
	}

	// try finding the child
	auto childIt = std::find_if(m_pChildren.begin(), m_pChildren.end(), [&searchName](Entry const* const entry)
		{
			return entry->GetNameOnly() == searchName;
		});

	if (childIt == m_pChildren.cend()) // child not found
	{
		return nullptr;
	}

	Entry* const childEntry = *childIt;

	if (!isFinalEntry) // if we are supposed to find a child of the found entry, make sure we have a directory and repeat the search in its children
	{
		if (childEntry->GetType() == Entry::EntryType::ENTRY_FILE)
		{
			return nullptr;
		}

		return static_cast<Directory*>(childEntry)->GetMountedChildRecursive(path);
	}

	// we found the final entry
	return childEntry;
}


} // namespace core
} // namespace et
