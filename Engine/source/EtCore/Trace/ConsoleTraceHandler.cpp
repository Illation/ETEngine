#include "stdafx.h"
#include "ConsoleTraceHandler.h"

#include <EtCore/Util/WindowsUtil.h>


namespace et {
namespace core {


//=======================
// Console Trace Handler
//=======================


//----------------------------
// ConsoleTraceHandler::d-tor
//
// Close the debug console
//
ConsoleTraceHandler::~ConsoleTraceHandler()
{
#ifdef ET_PLATFORM_WIN
	// Close the console
	if (!m_HadConsoleWindow)
	{
		// Close the streams - otherwise console won't close
		if (m_StdInPtr != nullptr)
		{
			fclose(m_StdInPtr.Get());
		}

		if (m_StdOutPtr != nullptr)
		{
			fclose(m_StdOutPtr.Get());
		}

		if (m_StdErrPtr != nullptr)
		{
			fclose(m_StdErrPtr.Get());
		}

		if (FreeConsole() == false)
		{
			DisplayError(TEXT("ConsoleTraceHandler::d-tor"));
		}
	}
#endif
}

//---------------------------------
// ConsoleTraceHandler::Initialize
//
// Open a console to output text to
//
bool ConsoleTraceHandler::Initialize()
{
#ifdef ET_PLATFORM_WIN
	// On Windows we don't start the engine as a console application, so we need to open it
	if (GetConsoleWindow() == NULL)
	{
		m_HadConsoleWindow = false;
		if (AllocConsole() == false)
		{
			DisplayError(TEXT("ConsoleTraceHandler::Initialize"));
			return false;
		}
	}
	else
	{
		m_HadConsoleWindow = true;
	}

	// Redirect the CRT standard input, output, and error handles to the console
	FILE* coutPtr;

	freopen_s(&coutPtr, "CONIN$", "r", stdin);
	m_StdInPtr = ToPtr(coutPtr);

	freopen_s(&coutPtr, "CONOUT$", "w", stdout);
	m_StdOutPtr = ToPtr(coutPtr);

	freopen_s(&coutPtr, "CONOUT$", "w", stderr);
	m_StdErrPtr = ToPtr(coutPtr);

	// clear error states on streams in case they where used before the console was allocated	
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
	std::cin.clear();

	// handle so we can change the text colour
	m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif

	m_OutStream = ToPtr(&std::cout);

	return true;
}

//-------------------------------------
// ConsoleTraceHandler::OnTraceMessage
//
// Print trace message to console
//
void ConsoleTraceHandler::OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message)
{
	// text color
#ifdef ET_PLATFORM_WIN
	switch (level)
	{
	case E_TraceLevel::TL_Warning:
		SetConsoleTextAttribute(m_ConsoleHandle, 14); // Yellow
		break;

	case E_TraceLevel::TL_Error:
	case E_TraceLevel::TL_Fatal:
		SetConsoleTextAttribute(m_ConsoleHandle, 12); // Red
		break;

	case E_TraceLevel::TL_Verbose:
	case E_TraceLevel::TL_Info:
	default:
		SetConsoleTextAttribute(m_ConsoleHandle, 15); // White
		break;
	}
#endif

	// message
	*m_OutStream << I_TraceHandler::BuildBasicMessage(context, level, timestamp, message);

	// display
	m_OutStream->flush();
}


} // namespace core
} // namespace et
