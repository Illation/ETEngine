#include "stdafx.h"
#include "Command.h"


namespace et {
namespace core {


std::string execConsoleCommand(std::string const& cmd)
{
	//modified version of https://stackoverflow.com/a/35935443/4414168
#ifdef PLATFORM_Win

	// Allocate 1Mo to store the output (final buffer will be sized to actual output)
	// If output exceeds that size, it will be truncated
	const SIZE_T RESULT_SIZE = sizeof(char) * 1024 * 1024;
	//char* output = (char*)LocalAlloc(LPTR, RESULT_SIZE);
	std::string output;

	HANDLE readPipe, writePipe;
	SECURITY_ATTRIBUTES security;
	STARTUPINFOA        start;
	PROCESS_INFORMATION processInfo;

	security.nLength = sizeof(SECURITY_ATTRIBUTES);
	security.bInheritHandle = true;
	security.lpSecurityDescriptor = NULL;

	if (CreatePipe( &readPipe, &writePipe, &security, 0)) 
	{
		GetStartupInfoA(&start);
		start.hStdOutput = writePipe;
		start.hStdError = writePipe;
		start.hStdInput = readPipe;
		start.dwFlags = STARTF_USESTDHANDLES + STARTF_USESHOWWINDOW;
		start.wShowWindow = SW_HIDE;

		std::string fullCommand = "cmd.exe /C " + cmd;
		LPSTR command = const_cast<char *>(fullCommand.c_str());

		// We have to start the DOS app the same way cmd.exe does (using the current Win32 ANSI code-page).
		// So, we use the "ANSI" version of createProcess, to be able to pass a LPSTR (single/multi-byte character string) 
		// instead of a LPWSTR (wide-character string) and we use the UNICODEtoANSI function to convert the given command 
		if (CreateProcessA(NULL,                    // pointer to name of executable module
			command,				 // pointer to command line string
			&security,               // pointer to process security attributes
			&security,               // pointer to thread security attributes
			TRUE,                    // handle inheritance flag
			NORMAL_PRIORITY_CLASS,   // creation flags
			NULL,                    // pointer to new environment block
			NULL,                    // pointer to current directory name
			&start,                  // pointer to STARTUPINFO
			&processInfo             // pointer to PROCESS_INFORMATION
		)) 
		{
			//// wait for the child process to start
			//for (UINT state = WAIT_TIMEOUT; state == WAIT_TIMEOUT; state = WaitForSingleObject(processInfo.hProcess, 100));

			auto WaitResult = WaitForSingleObject(processInfo.hProcess, INFINITE);
			if (WAIT_FAILED == WaitResult)
				throw std::runtime_error(("Waiting for batch process failed. Error code: " + std::to_string(GetLastError())).c_str());
			//else if (WAIT_TIMEOUT == WaitResult)

			DWORD bytesRead = 0, count = 0;
			const int BUFF_SIZE = 1024;
			char* buffer = (char*)malloc(sizeof(char)*BUFF_SIZE + 1);
			//strcpy(output, "");
			do 
			{
				DWORD dwAvail = 0;
				if (!PeekNamedPipe(readPipe, NULL, 0, NULL, &dwAvail, NULL)) {
					// error, the child process might have ended
					break;
				}
				if (!dwAvail) {
					// no data available in the pipe
					break;
				}
				ReadFile(readPipe, buffer, BUFF_SIZE, &bytesRead, NULL);
				buffer[bytesRead] = '\0';
				if ((count + bytesRead) > RESULT_SIZE) break;
				//strcat(output, buffer);
				output += buffer;
				count += bytesRead;
			} 
			while (bytesRead >= BUFF_SIZE);
			free(buffer);
		}

		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	}
	CloseHandle(writePipe);
	CloseHandle(readPipe);

	// convert result buffer to a wide-character string
	return output;
#else
	// #todo linux version
	return std::string("execConsoleCommand: not implemented");
#endif
}


} // namespace core
} // namespace et
