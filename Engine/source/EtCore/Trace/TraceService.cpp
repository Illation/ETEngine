#include "stdafx.h"
#include "TraceService.h"

#ifdef ET_PLATFORM_WIN
#	include <EtCore/Util/WindowsUtil.h>
#endif


namespace et {
namespace core {


//====================================
// Trace Service :: Context Container
//====================================


//-----------------------------------
// ContextContainer::RegisterContext
//
T_TraceContext TraceService::ContextContainer::RegisterContext(std::string const& name)
{
	T_TraceContext const ret = GetHash(name);

	if (m_Contexts.find(ret) != m_Contexts.cend())
	{
		ET_BREAK(); // Can't reliably use asserts because contexts will be registered before asserts are initialized
		return 0u;
	}

	m_Contexts.emplace(ret, name);
	return ret;
}

//----------------------------------
// ContextContainer::GetContextName
//
std::string const& TraceService::ContextContainer::GetContextName(T_TraceContext const hash)
{
	static std::string const s_InvalidContextName = "invalid_trace_context";

	auto const foundIt = m_Contexts.find(hash);
	if (foundIt == m_Contexts.cend())
	{
		ET_ASSERT(false, "invalid trace context name!");
		return s_InvalidContextName;
	}

	return foundIt->second;
}


//===============
// Trace Service
//===============


RefPtr<TraceService> TraceService::s_Instance = nullptr;


//--------------------------
// TraceService::Initialize
//
void TraceService::Initialize()
{
	ET_ASSERT(!IsInitialized());
	s_Instance = Create<TraceService>();
}

//---------------------------
// TraceService::GetInstance
//
void TraceService::Destroy()
{
	if (IsInitialized())
	{
		ET_ASSERT(s_Instance.GetRefCount() == 1, "Singleton destruction won't deinit the trace service");
		s_Instance = nullptr;
	}
}

//-----------------------------
// TraceService::IsInitialized
//
bool TraceService::IsInitialized()
{
	return (s_Instance != nullptr);
}

//-----------------------------
// TraceService::IsInitialized
//
TraceService::ContextContainer& TraceService::GetContextContainer()
{
	static ContextContainer s_ContextContainer;
	return s_ContextContainer;
}

//---------------------
// TraceService::c-tor
//
TraceService::TraceService()
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
}

//---------------------
// TraceService::d-tor
//
TraceService::~TraceService()
{
	StopFileLogging();
}

//---------------------
// TraceService::Trace
//
// For now we handle tracing in the same way the engine has before
//
void TraceService::Trace(T_TraceContext const context, E_TraceLevel const level, bool const timestamp, std::string const& msg)
{
#if ET_CT_IS_DISABLED(ET_CT_TRACE_VERBOSE)
	if (level & E_TraceLevel::TL_Verbose)
	{
		return;
	}
#endif

	// Generate complete message
	//---------------------------

	// timestamp
	std::stringstream timestampStream;
	bool genTimestamp = timestamp || (m_FileStream != nullptr);
#if ET_CT_IS_ENABLED(ET_CT_TRACE_DBG_OUT)
#ifdef ET_PLATFORM_WIN
	if (IsDebuggerPresent())
	{
		genTimestamp = true;
	}
#endif
#endif

	if (genTimestamp)
	{
		SYSTEMTIME st;
		GetSystemTime(&st);

		timestampStream << "[";
		if (m_AddDate)
		{
			timestampStream << st.wYear << "/" << st.wMonth << "/" << st.wDay << "-";
		}

		timestampStream << st.wHour << "." << st.wMinute << "." << st.wSecond << ":" << st.wMilliseconds << "]";
	}

	std::stringstream stream;

	// trace level
	switch (level)
	{
	case E_TraceLevel::TL_Verbose:
		stream << "[VERBOSE] ";
		break;

	case E_TraceLevel::TL_Info:
		break;

	case E_TraceLevel::TL_Warning:
		stream << "[WARNING] ";
		break;

	case E_TraceLevel::TL_Error:
		stream << "[ERROR] ";
		break;

	case E_TraceLevel::TL_Fatal:
		stream << "[FATAL] ";
		break;

	default:
		ET_ASSERT(false, "Unhandled trace level");
		break;
	}

	// the context
	stream << "[" << GetContextContainer().GetContextName(context) << "] ";

	// the message
	stream << msg;
	stream << "\n";

	timestampStream << stream.str();

	// Write to console
	//------------------

	// text color
#ifdef ET_PLATFORM_WIN
	switch (level)
	{
	case E_TraceLevel::TL_Verbose:
	case E_TraceLevel::TL_Info:
	default:
		SetConsoleTextAttribute(m_ConsoleHandle, 15); // White
		break;

	case E_TraceLevel::TL_Warning:
		SetConsoleTextAttribute(m_ConsoleHandle, 14); // Yellow
		break;

	case E_TraceLevel::TL_Error:
	case E_TraceLevel::TL_Fatal:
		SetConsoleTextAttribute(m_ConsoleHandle, 12); // Red
		break;
	}
#endif

	if (timestamp)
	{
		*m_OutStream << timestampStream.str();
	}
	else
	{
		*m_OutStream << stream.str();
	}

	m_OutStream->flush();

	// Write to file
	//---------------
	if (m_FileStream != nullptr)
	{
		*m_FileStream << timestampStream.str();
		m_FileStream->flush();
	}

	// Write to debug out
	//--------------------
#if ET_CT_IS_ENABLED(ET_CT_TRACE_DBG_OUT)
#ifdef ET_PLATFORM_WIN
	if (IsDebuggerPresent())
	{
		OutputDebugString(timestampStream.str().c_str());
	}
#endif
#endif

	// On error
	//----------
	static T_TraceLevel const s_BreakBitField = TL_Error | TL_Fatal;
	if (level & s_BreakBitField)
	{
#ifdef ET_PLATFORM_WIN // on windows we show a message box for errors
		if (IsDebuggerPresent())
		{
			ET_BREAK();
		}
		else
		{
			MessageBox(0, msg.c_str(), "ERROR", 0);
		}
#else
		ET_BREAK();
#endif // ET_PLATFORM_WIN
	}

	// crash out of the program before we do more damage
	if (level & TL_Fatal) 
	{
		exit(-1);
	}
}

//--------------------------------
// TraceService::StartFileLogging
//
void TraceService::StartFileLogging(std::string const& fileName)
{
	if (m_FileStream != nullptr)
	{
		m_FileStream->close();
	}

	m_FileStream = Create<std::ofstream>(fileName.c_str());
}

//-------------------------------
// TraceService::StopFileLogging
//
void TraceService::StopFileLogging()
{
	if (m_FileStream != nullptr)
	{
		m_FileStream->close();
		m_FileStream = nullptr;
	}
}


} // namespace core
} // namespace et
