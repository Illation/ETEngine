#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <EtCore/Platform/PlatformUtil.h>


namespace et {
namespace core {


FILE_HANDLE FILE_BASE::Open( const char * pathName, FILE_ACCESS_FLAGS accessFlags, FILE_ACCESS_MODE accessMode )
{
	uint32 mode = 0;
	uint32 share = 0;
	switch (accessMode)
	{
	case Read:
		mode |= GENERIC_READ;
		share = FILE_SHARE_READ;
		break;
	case Write:
		mode |= GENERIC_WRITE;
		break;
	case ReadWrite:
		mode |= GENERIC_READ | GENERIC_WRITE;
		break;
	}
	uint32 flags = OPEN_ALWAYS;//might need to revise those flags meanings
	if (accessFlags.GetFlag(FILE_ACCESS_FLAGS::FLAGS::Truncate)
		&& accessFlags.GetFlag(FILE_ACCESS_FLAGS::FLAGS::Exists))
	{
		flags = TRUNCATE_EXISTING;
	}
	else if (accessFlags.GetFlag(FILE_ACCESS_FLAGS::FLAGS::Create)
		&& !accessFlags.GetFlag(FILE_ACCESS_FLAGS::FLAGS::Exists))
	{
		flags = CREATE_NEW;
	}
	else if (accessFlags.GetFlag(FILE_ACCESS_FLAGS::FLAGS::Create))
	{
		flags = CREATE_ALWAYS;
	}
	else if (accessFlags.GetFlag(FILE_ACCESS_FLAGS::FLAGS::Exists))
	{
		flags = OPEN_EXISTING;
	}

	FILE_HANDLE ret = CreateFile( std::string( pathName ).c_str(), mode, share, NULL, flags, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( ret == INVALID_HANDLE_VALUE)
	{
		platform::DisplayLastError("CreateFile");
	}

	return ret;
}

bool FILE_BASE::Close( FILE_HANDLE handle )
{
	BOOL result = CloseHandle(handle);
	if(result == FALSE)
	{
		platform::DisplayLastError("DeleteFile");
		return false;
	}

	return true;
}

bool FILE_BASE::GetEntrySize(FILE_HANDLE handle, int64& size)
{
	LARGE_INTEGER largeSize;
	BOOL result = GetFileSizeEx(handle, &largeSize);

	size = -1;

	if (result == FALSE)
	{
		platform::DisplayLastError("GetFileSizeEx");
		return false;
	}

	size = largeSize.QuadPart;

	return true;
}

bool FILE_BASE::ReadFile( FILE_HANDLE handle, std::vector<uint8> & content, uint64 const numBytes, uint64 const offset)
{
	char* buffer_read = new char[static_cast<DWORD>(numBytes) + 1];
	DWORD bytes_read = 0;

	OVERLAPPED ov = {};
	ov.Offset = static_cast<DWORD>(offset);

	if (FALSE == ::ReadFile(handle, buffer_read, static_cast<DWORD>(numBytes), &bytes_read, &ov))
	{
		platform::DisplayLastError("ReadFile");
		delete[] buffer_read;
		return false;
	}

	if (bytes_read >= 0 && bytes_read <= static_cast<DWORD>(numBytes)) //move buffer to string
	{
		buffer_read[bytes_read] = '\0'; // NULL character
		content = std::vector<uint8>(buffer_read, buffer_read + bytes_read);
		delete[] buffer_read;
		return true;
	}

	delete[] buffer_read;
	return false;
}

bool FILE_BASE::WriteFile( FILE_HANDLE handle, const std::vector<uint8> & content)
{
	DWORD bytesWritten = 0;
	if (FALSE == ::WriteFile(handle, content.data(), (DWORD)content.size(), &bytesWritten, NULL))
	{
		platform::DisplayLastError("WriteFile");
		return false;
	}
	else if (bytesWritten != content.size()) return false;

	if (FALSE == ::FlushFileBuffers(handle))
	{
		platform::DisplayLastError("WriteFile->FlushFileBuffers");
		return false;
	}

	return true;
}

bool FILE_BASE::DeleteFile( const char * pathName )
{
	BOOL result = ::DeleteFile( pathName );
	if(result == FALSE)
	{
		platform::DisplayLastError("DeleteFile");
		return false;
	}

	return true;
}

bool FILE_BASE::Exists(char const* fileName)
{
	DWORD dwAttrib = GetFileAttributes(fileName);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}


} // namespace core
} // namespace et
