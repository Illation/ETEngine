#include "stdafx.h"

#ifdef ET_PLATFORM_WIN

#include "PlatformUtil.h"

#include <iostream>
#include <tchar.h>
#include <stdio.h>
#include <strsafe.h>
#include <minwindef.h>
#include <comutil.h>

#ifndef WIN32_LEAN_AND_MEAN
#	define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tlhelp32.h>
#include <shellapi.h>


namespace et {
namespace core {

namespace platform {


// static
std::string const Util::s_ExecutableExtension = ".exe";


//---------------------
// DisplayLastError
//
// Displays the last error in the winApi. lpszFunction is just an argument that enhances the debug output
//
void DisplayLastError(std::string const& message)
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

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + message.size() + 40) /* account for format string */ * sizeof(TCHAR));

	if (FAILED(StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error code %d as follows:\n%s"),
		message.c_str(),
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

//--------------------
// CommandLineToArgvA
//
// wide string conversion
//
std::vector<std::string> CommandLineToArgvA()
{
	std::vector<std::string> ret;

	int nArgs;
	LPWSTR* szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);

	for (int i = 0; i < nArgs; ++i)
	{
		std::wstring const ws(szArglist[i]);
		std::string str(ws.length(), 0);
		std::transform(ws.begin(), ws.end(), str.begin(), [](wchar_t c) 
			{
				return static_cast<char>(c);
			});
		ret.push_back(str);
	}

	LocalFree(szArglist);

	return ret;
}

//---------------------------------
// GetCommandlineArgV
//
// convert the WinMain style command line into regular c++ entry point arguments
//
std::vector<char*> GetCommandlineArgV()
{
	// static so that we don't loose the memory of the char* - this only needs to be called once
	static std::vector<std::string> separatedCmdLine = CommandLineToArgvA();

	std::vector<char*> argv;
	argv.reserve(separatedCmdLine.size());

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

//---------------------------------
// ListRunningProcesses
//
// based on https://stackoverflow.com/a/42126277/4414168
//
std::vector<std::string> ListRunningProcesses()
{
	std::vector<std::string> ret;

	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnap != INVALID_HANDLE_VALUE) 
	{
		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (Process32First(hProcessSnap, &pe32)) 
		{ 
			// Gets first running process
			ret.push_back(std::string(pe32.szExeFile));

			// loop through all running processes looking for process
			while (Process32Next(hProcessSnap, &pe32)) 
			{
				ret.push_back(std::string(pe32.szExeFile));
			}

			// clean the snapshot object
			CloseHandle(hProcessSnap);
		}
	}

	return ret;
}

//--------------------
// IsDebuggerAttached
//
bool IsDebuggerAttached()
{
	return IsDebuggerPresent();
}


} // namespace platform

} // namespace core
} // namespace et

#endif // def ET_PLATFORM_WIN
