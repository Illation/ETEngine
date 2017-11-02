#include "stdafx.hpp"

#include "Entry.h"

#include <iostream>

#include "./Facade/FileBase.h"
#include "./Facade/FileAccessMode.h"
#include "./Facade/FileAccessFlags.h"

Entry::Entry(std::string name, Directory* pParent)
	:m_Filename(name)
	,m_pParent(pParent)
{
}
std::string Entry::GetPath()
{
	if(m_pParent)return std::string(m_pParent->GetPath()+m_pParent->GetName());
	else return "./";
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
	m_Handle = FILE_BASE::Open(m_Filename.c_str(), flags, mode);
	if (m_Handle == FILE_HANDLE_INVALID) 
	{
		std::cerr << "open";
		return false;
	}
	m_IsOpen = true;
	return true;
}
std::string File::Read()
{
	std::string content;
	if(!FILE_BASE::ReadFile(m_Handle, content))
	{
		return "";
	}
	return content;
}
bool File::Write(const std::string &lhs)
{
	if ( !FILE_BASE::WriteFile(m_Handle, lhs) )
    {
        std::cerr << "write";
        return false;
    }
	return true;
}
void File::Close()
{
	FILE_BASE::Close(m_Handle);
	m_IsOpen = false;
}

Directory::Directory(std::string name, Directory* pParent)
	:Entry(name, pParent)
{
}
Directory::~Directory()
{
    if(m_IsMounted)Unmount();
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
