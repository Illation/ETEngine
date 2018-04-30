#include "stdafx.hpp"

#include "Entry.h"

#include <iostream>

#include "./Facade/FileBase.h"
#include "./Facade/FileAccessMode.h"
#include "./Facade/FileAccessFlags.h"
#include "FileUtil.h"

Entry::Entry(std::string name, Directory* pParent)
	:m_Filename(name)
	,m_pParent(pParent)
{
	if (!m_pParent)
	{
		m_Path = FileUtil::ExtractPath(m_Filename);
		m_Filename = FileUtil::ExtractName(m_Filename);
	}
	else m_Path = "./";
}
std::string Entry::GetPath()
{
	if(m_pParent)return std::string(m_pParent->GetPath()+m_pParent->GetNameOnly());
	else return m_Path;
}

std::string Entry::GetName()
{
	return m_Path + m_Filename;
}

std::string Entry::GetNameOnly()
{
	return m_Filename;
}

std::string Entry::GetExtension()
{
	if(GetType() == EntryType::ENTRY_FILE)
	{
		std::size_t found = m_Filename.rfind(".");
		if(found != std::string::npos)
			return m_Filename.substr(found+1);
	}
	return std::string("");
}

File::File(std::string name, Directory* pParent)
	:Entry(name, pParent)
	,m_IsOpen(false)
{
	
}
File::~File()
{
	if(m_IsOpen)Close();
}
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
	if(m_pParent)
	{
		if(!(std::find( m_pParent->m_pChildren.begin(), m_pParent->m_pChildren.end(), this ) != m_pParent->m_pChildren.end()))
		{
			m_pParent->m_pChildren.push_back( this );
		}
	}
	return true;
}
std::vector<uint8> File::Read()
{
	std::vector<uint8> content;
	if(!FILE_BASE::ReadFile(m_Handle, content))
	{
		LOG("Reading File failed", Error);
	}
	return content;
}
bool File::Write(const std::vector<uint8> &lhs)
{
	if ( !FILE_BASE::WriteFile(m_Handle, lhs) )
	{
		LOG("Writing File failed", Error);
        return false;
    }
	return true;
}
void File::Close()
{
	if(FILE_BASE::Close( m_Handle ))
	{
		m_IsOpen = false;
	}
}
bool File::Delete()
{
	if(m_IsOpen)
		Close();

	if(m_IsOpen)
	{
		LOG("Couldn't delete file because it failed to close", Error);
		return false;
	}

	std::string path = GetPath()+m_Filename;
	if(FILE_BASE::DeleteFile( path.c_str() ))
	{
		if(m_pParent)
		{
			m_pParent->RemoveChild( this );
		}
		delete this;
		return true;
	}
	return false;
}

Directory::Directory(std::string name, Directory* pParent, bool ensureExists)
	:Entry(name, pParent)
{
	if (ensureExists) Create();
}
Directory::~Directory()
{
    if(m_IsMounted)Unmount();
}

void Directory::RemoveChild( Entry* child )
{
	m_pChildren.erase( 
		std::remove( m_pChildren.begin(), m_pChildren.end(), child )
		, m_pChildren.end() );
}

void Directory::RecursiveMount()
{
    for(auto c : m_pChildren)
    {
        if(c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
        {
			if(c->GetName() != "../" && c->GetName() != "./")
				static_cast<Directory*>(c)->Mount(true); 
        }
    }
}
void Directory::Unmount()
{
    for(auto c : m_pChildren)
    {
        if(c->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
        {
			if(c->GetName() != "../" && c->GetName() != "./")
				static_cast<Directory*>(c)->Unmount();
        }
		delete c;
		c = nullptr;
    }
    m_pChildren.clear();
    m_IsMounted = false;
}
std::vector<Entry*> Directory::GetChildrenByExt(std::string ext)
{
	std::vector<Entry*> ret;
	for(auto e : m_pChildren)
		if(e->GetExtension()==ext || e->GetType() == Entry::EntryType::ENTRY_DIRECTORY)
			ret.push_back(e);
	return ret;
}

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
		if(m_pParent)
		{
			m_pParent->RemoveChild( this );
		}
		delete this;
		return true;
	}
	return false;
}
