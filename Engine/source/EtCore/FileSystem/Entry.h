#pragma once

#include <string>
#include <vector>

#include "./Facade/FileHandle.h"
#include "./Facade/FileAccessFlags.h"
#include "./Facade/FileAccessMode.h"


namespace et {
namespace core {


class Directory;

//---------------------------------
// Entry
//
// Content of any kind that lives in a directory
//
class Entry
{
public:
	Entry(std::string name, Directory* pParent);
    virtual ~Entry();

    std::string GetName() const;
	std::string const& GetNameOnly() const { return m_Filename; }
	std::string GetExtension();
    Directory* GetParent(){ return m_Parent; }
    std::string GetPath() const;
	bool IsChildOf(Directory const* const dir) const;

	virtual bool Exists() = 0;
	virtual bool Delete() = 0;

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
    Directory* m_Parent;
	std::string m_Path; //only used if there is no parent
};

//---------------------------------
// File
//
// Entry that contains data
//
class File : public Entry
{
public:
	File(std::string name, Directory* pParent);
	virtual ~File();

	bool Open(FILE_ACCESS_MODE mode, 
		FILE_ACCESS_FLAGS flags = FILE_ACCESS_FLAGS());
	void Close();

	std::vector<uint8> Read();
	std::vector<uint8> ReadChunk(uint64 const offset, uint64 const numBytes);
	bool Write(const std::vector<uint8> &lhs);
	Entry::EntryType GetType()
    	{
            return Entry::EntryType::ENTRY_FILE;
        }

	bool IsOpen(){ return m_IsOpen; }

	uint64 GetSize();

	bool Exists() override;
	bool Delete() override;

private:
	bool m_IsOpen;

	FILE_HANDLE m_Handle;
};

//---------------------------------
// Directory
//
// Entry that contains more entries
//
class Directory : public Entry
{
	friend class File;
	//friend class Directory;

public:
    // construct destruct
	//--------------------
    Directory(std::string name, Directory* pParent, bool ensureExists = false);
    virtual ~Directory();

    // Entry interface
	//------------------
    Entry::EntryType GetType() override { return Entry::EntryType::ENTRY_DIRECTORY; }
    
	// functionality
	//---------------
    bool Mount(bool recursive = false);
    void Unmount();

	bool Create();

	bool Exists() override;
	bool Delete() override;

	void RemoveChild(Entry* child);

	core::Directory* CreateSubdirectory(std::string& path);

	// accessors
	//---------------
	std::vector<Entry*> const& GetChildren() { return m_pChildren; }
	std::vector<Entry*> GetChildrenByExt(std::string const& ext);
	void GetChildrenRecursive(std::vector<File*>& children);

	Entry* GetMountedChild(std::string const& path) const;

	bool IsMounted() { return m_IsMounted; }

	// utility
	//-----------
private:

	bool DeleteDir();
    void RecursiveMount();
	Entry* GetMountedChildRecursive(std::string& path) const;

	// Data
	///////

    std::vector<Entry*> m_pChildren;
    bool m_IsMounted = false;
};


} // namespace core
} // namespace et
