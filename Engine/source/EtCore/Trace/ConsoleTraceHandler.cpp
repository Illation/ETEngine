#include "stdafx.h"
#include "ConsoleTraceHandler.h"


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

}

//---------------------------------
// ConsoleTraceHandler::Initialize
//
// Open a console to output text to
//
bool ConsoleTraceHandler::Initialize()
{
	// Check if we already have a console attached
	//if (!_isatty(_fileno(stdout)))
	//{
	//	// if not, create one
	//	if (!AllocConsole())
	//	{
	//		std::cout << "Warning: Could not attach to console" << std::endl;
	//		CheckBreak(Error);
	//		return;
	//	}
	//}

	// Redirect the CRT standard input, output, and error handles to the console
	FILE* coutPtr;
	freopen_s(&coutPtr, "CONIN$", "r", stdin);
	freopen_s(&coutPtr, "CONOUT$", "w", stdout);
	freopen_s(&coutPtr, "CONOUT$", "w", stderr);

	//Clear the error state for each of the C++ standard stream objects. We need to do this, as
	//attempts to access the standard streams before they refer to a valid target will cause the
	//iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
	//to always occur during startup regardless of whether anything has been read from or written to
	//the console or not.
	std::wcout.clear();
	std::cout.clear();
	std::wcerr.clear();
	std::cerr.clear();
	std::wcin.clear();
	std::cin.clear();
	std::cin.clear();

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
