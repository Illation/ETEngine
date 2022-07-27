#include "stdafx.h"
#include "WindowsUtil.h"

#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <minwindef.h>
#include <comutil.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>


namespace et {
namespace core {


//---------------------
// DisplayError
//
// Displays the last error in the winApi. lpszFunction is just an argument that enhances the debug output
//
void DisplayError(LPTSTR lpszFunction)
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);

	lpDisplayBuf =
		(LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf)
			+ lstrlen((LPCTSTR)lpszFunction)
			+ 40) // account for format string
			* sizeof(TCHAR));

	if (FAILED(StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error code %d as follows:\n%s"),
		lpszFunction,
		dw,
		lpMsgBuf)))
	{
		ET_FATAL("Unable to output error code");
	}

	ET_LOG_E(ET_CTX_CORE, (LPCTSTR)lpDisplayBuf);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
}

//---------------------
// GetExecutablePathName
//
// Gets the name and path the executable lives in
//
void GetExecutablePathName(std::string& outPath)
{
	char ownPth[MAX_PATH];

	// When NULL is passed to GetModuleHandle, the handle of the exe itself is returned
	HMODULE hModule = GetModuleHandle(NULL);
	ET_ASSERT(hModule != NULL);

	// Use GetModuleFileName() with module handle to get the path
	GetModuleFileName(hModule, ownPth, (sizeof(ownPth)));

	outPath = std::string(ownPth);
}


} // namespace core
} // namespace et
