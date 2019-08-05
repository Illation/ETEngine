#include "stdafx.h"
#include "Entry.h"

#include <iostream>
#include <limits>

#include "FileUtil.h"

#include <EtCore/FileSystem/Facade/FileBase.h>
#include <EtCore/FileSystem/Facade/FileAccessMode.h>
#include <EtCore/FileSystem/Facade/FileAccessFlags.h>


//======================
// Entry
//======================


//---------------------------------
// Entry::Entry
//
// Entry constructor sets up the parent and path members
//
Entry::Entry(std::string name, Directory* pParent)
	: m_Filename(name)
	, m_Parent(pParent)
{
	if (m_Parent == nullptr)
	{
		m_Path = FileUtil::ExtractPath(m_Filename);
		if (!FileUtil::IsAbsolutePath(m_Path))
		{
			m_Path = FileUtil::GetAbsolutePath(m_Path);
		}

		m_Filename = FileUtil::ExtractName(m_Filename);
	}
	else
	{
		m_Path = "/";
	}
}

//---------------------------------
// Entry::GetPath
//
// Get the path of an entry, recursively stepping up the parent list
//
std::string Entry::GetPath()
{
	if (m_Parent)
	{
		return std::string(m_Parent->GetPath() + m_Parent->GetNameOnly());
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
std::string Entry::GetName()
{
	return m_Path + m_Filename;
}

//---------------------------------
// Entry::GetNameOnly
//
// Get the name of an entry without the attached path
//
std::string Entry::GetNameOnly()
{
	return m_Filename;
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
		std::size_t found = m_Filename.rfind(".");
		if (found != std::string::npos)
		{
			return m_Filename.substr(found+1);
		}
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
		LOG("Opening File failed", Error);
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
		LOG("File::Read > Reading File failed", Error);
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
		LOG("File::ReadChunk > Reading file failed", Error);
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
		LOG("Writing File failed", Error);
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
		LOG("Getting File size failed", Error);
		return std::numeric_limits<uint64>::max();
	}

	return static_cast<uint64>(size);
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
		LOG("Couldn't delete file because it failed to close", Error);
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
// Directory::RecursiveMount
//
// Recursively mount all subdirectories
//
void Directory::RecursiveMount()
{
    for(auto c : m_pChildren)
    {
        if(c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
        {
			if (c->GetName() != "../" && c->GetName() != "./")
			{
				static_cast<Directory*>(c)->Mount(true); 
			}
        }
    }
}

//---------------------------------
// Directory::Unmount
//
// Unmount the directory and all subdirectories
//
void Directory::Unmount()
{
    for(auto c : m_pChildren)
    {
        if(c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
        {
			if (c->GetName() != "../" && c->GetName() != "./")
			{
				static_cast<Directory*>(c)->Unmount();
			}
        }

		delete c;
		c = nullptr;
    }

    m_pChildren.clear();
    m_IsMounted = false;
}

//---------------------------------
// Directory::GetChildrenByExt
//
// Get all children by file extension
//
std::vector<Entry*> Directory::GetChildrenByExt(std::string ext)
{
	std::vector<Entry*> ret;
	for (auto e : m_pChildren)
	{
		if (e->GetExtension() == ext || e->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
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
			LOG("Directory::GetChildrenRecursive > Unhandled entry type: " + child->GetName(), LogLevel::Warning);
			break;
		}
	}
}

//---------------------------------
// Directory::Delete
//
// Delete this directory, also deletes the object
//
bool Directory::Delete()
{
    for(auto c : m_pChildren)
    {
        if(c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
        {
			if(c->GetName() != "../" && c->GetName() != "./")
			{
				if(!(c->Delete()))return false;
			}
        }
		else
		{
			if(!(c->Delete()))return false;
		}

		c = nullptr;
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
