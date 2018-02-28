#include <sys/types.h>
#include <windows.h>

#include "../../Helper/WindowsUtil.h"

bool Directory::Exists()
{
	std::string path = GetPath() + m_Filename;

	DWORD dwAttrib = GetFileAttributes(path.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool Directory::Create()
{
	if (Exists())
	{
		return true;
	}
	std::string path = GetPath() + m_Filename;
	if (!CreateDirectory(path.c_str(), NULL))
	{
		DisplayError(TEXT("CreateDirectory"));
		return false;
	}
	return true;
}

bool Directory::Mount(bool recursive)
{
    if(!m_IsMounted)
    {
		HANDLE hFind;
		WIN32_FIND_DATA data;

		std::string path = GetPath()+m_Filename;
		if (path.substr(0, 2) == "./")path = path.substr(2);

		std::string target = std::string(path.begin(), path.end()) + std::string("*.*");

		hFind = FindFirstFile(target.c_str(), &data);
		if (hFind != INVALID_HANDLE_VALUE)
		{
			do
			{
				std::string name = data.cFileName;
				if (name != "." && name != "..")
				{
					Entry* e;
					if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						e = new Directory(std::string(name.begin(), name.end()) + "/", this);
					}
					else
					{
						e = new File(std::string(name.begin(), name.end()), this);
					}
					m_pChildren.push_back(e);
				}
			}
			while (FindNextFile(hFind, &data));
			FindClose(hFind);
		}
    }
    if(recursive)RecursiveMount();
    m_IsMounted = true;
    return true;
}

bool Directory::DeleteDir()
{
	std::string path = GetPath()+m_Filename;
	return ::RemoveDirectory( path.c_str() ) == TRUE;
}