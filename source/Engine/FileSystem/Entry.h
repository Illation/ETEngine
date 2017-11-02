#pragma once

#include <string>
#include <vector>

#include "./Facade/FileHandle.h"
#include "./Facade/FileAccessFlags.h"
#include "./Facade/FileAccessMode.h"

class Directory;

class Entry
{
public:
	Entry(std::string name, Directory* pParent);
    virtual ~Entry(){}

    std::string GetName(){ return m_Filename; }
	std::string GetExtension();
    Directory* GetParent(){ return m_pParent; }
    virtual std::string GetPath();

    enum EntryType
    {
        ENTRY_FILE,
        ENTRY_DIRECTORY,
        ENTRY_UNDEF
    };
    virtual EntryType GetType()
    {
        return Entry::EntryType::ENTRY_UNDEF; 
    }
protected:
	std::string m_Filename;
    Directory* m_pParent;
};

class File : public Entry
{
public:
	File(std::string name, Directory* pParent);
	virtual ~File();

	bool Open(FILE_ACCESS_MODE mode, 
		FILE_ACCESS_FLAGS flags = FILE_ACCESS_FLAGS());
	void Close();

	std::string Read();
	bool Write(const std::string &lhs);
	Entry::EntryType GetType()
    	{
            return Entry::EntryType::ENTRY_FILE;
        }

	bool IsOpen(){ return m_IsOpen; }

    
private:
	bool m_IsOpen;

	FILE_HANDLE m_Handle;
};

class Directory : public Entry
{
public:
    Directory(std::string name, Directory* pParent);
    virtual ~Directory();
    //Inherited Methods
    Entry::EntryType GetType()
    {
        return Entry::EntryType::ENTRY_DIRECTORY;
    }
    
    bool Mount(bool recursive = false);
    void Unmount();
    std::vector<Entry*> GetChildren(){return m_pChildren;}
	std::vector<Entry*> GetChildrenByExt(std::string ext);

    bool IsMounted(){ return m_IsMounted; }
private:
    void RecursiveMount();
    std::vector<Entry*> m_pChildren;
    bool m_IsMounted = false;
};
