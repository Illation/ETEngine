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

//---------------------------------
// GetCommandlineArgV
//
// convert the WinMain style command line into regular c++ entry point arguments
//
std::vector<char*> GetCommandlineArgV()
{
	// static so that we don't loose the memory of the char* - this only needs to be called once
	static std::vector<std::string> separatedCmdLine = SeparateByWhitespace(GetCommandLine()); 

	std::vector<char*> argv;
	for (std::string& el : separatedCmdLine)
	{
		argv.push_back(&el[0]);
	}

	return argv;
}

//---------------------------------
// LaunchExecutable
//
void LaunchExecutable(std::string const& path)
{
	// additional information
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	// set the size of the structures
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	// start the program up
	CreateProcess(path.c_str(),   // the path
		NULL,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
	);

	// Close process and thread handles. 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
}


} // namespace core
} // namespace et
