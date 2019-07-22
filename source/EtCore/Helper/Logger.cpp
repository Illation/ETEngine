#include "stdafx.h"
#include "Logger.h"

#ifdef PLATFORM_Win
#include "WindowsUtil.h"
#endif

Logger::ConsoleLogger* Logger::m_ConsoleLogger = nullptr;
Logger::FileLogger* Logger::m_FileLogger = nullptr;
Logger::DebugLogger* Logger::m_DebugLogger = nullptr;
uint8 Logger::m_BreakBitField = LogLevel::Error;
bool Logger::m_TimestampDate = true;
bool Logger::m_IsInitialized = false;

void Logger::Initialize()
{
	m_ConsoleLogger = new ConsoleLogger();
#ifndef SHIPPING
	if (IsDebuggerPresent())
	{
		m_DebugLogger = new DebugLogger();
	}
#endif
	m_IsInitialized = true;
}

void Logger::Release()
{
	SafeDelete(m_ConsoleLogger);
	SafeDelete(m_FileLogger);
	SafeDelete(m_DebugLogger);
}

void Logger::StartFileLogging(const std::string& fileName)
{
	SafeDelete(m_FileLogger);

	m_FileLogger = new FileLogger(fileName);
}

void Logger::StopFileLogging()
{
	SafeDelete(m_FileLogger);
}

ivec2 Logger::GetCursorPosition()
{
	if (m_ConsoleLogger)
	{
		return m_ConsoleLogger->GetCursorPosition();
	}

	return ivec2(-1);
}

void Logger::Log(const std::string& msg, LogLevel level, bool timestamp, ivec2 cursorPos)
{
#ifndef _DEBUG
	if (level&Verbose)return;
#endif

	std::stringstream stream;

	std::stringstream timestampStream;
	bool genTimestamp = timestamp || m_FileLogger;
#ifndef SHIPPING
#ifdef PLATFORM_Win
	if (IsDebuggerPresent())genTimestamp = true;
#endif
#endif
	if(genTimestamp)
	{
		SYSTEMTIME st;
		GetSystemTime(&st);

		timestampStream << "[";
		if (m_TimestampDate)
		{
			timestampStream << st.wYear << "/" << st.wMonth << "/" << st.wDay << "-";
		}

		timestampStream << st.wHour << "." << st.wMinute << "." << st.wSecond << ":" << st.wMilliseconds << "]";
	}

	switch (level)
	{
	case LogLevel::Info:
		break;
	case LogLevel::Warning:
		stream << "[WARNING] ";
		break;
	case LogLevel::Error:
		stream << "[ERROR]   ";
		break;
	case LogLevel::FixMe:
		stream << "[FIX-ME]   ";
		break;
	}
	stream << msg;
	stream << "\n";

	timestampStream << stream.str();

	//Use specific loggers to log
	if (m_ConsoleLogger)
	{
		// on non shipping builds we set the color in the console
#ifndef SHIPPING
		switch (level)
		{
		case LogLevel::Info: m_ConsoleLogger->SetColor(ConsoleLogger::Color::WHITE); break;
		case LogLevel::Warning: m_ConsoleLogger->SetColor(ConsoleLogger::Color::YELLOW); break;
		case LogLevel::Error: m_ConsoleLogger->SetColor(ConsoleLogger::Color::RED); break;
		case LogLevel::FixMe: m_ConsoleLogger->SetColor(ConsoleLogger::Color::MAGENTA); break;
		}
#endif // SHIPPING

		if (!(cursorPos == ivec2(-1)))
		{
			m_ConsoleLogger->SetCursorPosition(cursorPos);
		}

		if (timestamp)
		{
			m_ConsoleLogger->Log(timestampStream.str());
		}
		else
		{
			m_ConsoleLogger->Log(stream.str());
		}
	}

	if (m_FileLogger)
	{
		if (!(cursorPos == ivec2(-1)))
		{
			m_FileLogger->SetCursorPosition(cursorPos);
		}

		m_FileLogger->Log(timestampStream.str());
	}

#ifndef SHIPPING
	if (m_DebugLogger) // on non shipping builds we also log to the vis
	{
		if (!(cursorPos == ivec2(-1)))
		{
			m_DebugLogger->SetCursorPosition(cursorPos);
		}

		m_DebugLogger->Log(timestampStream.str());
	}

#ifdef PLATFORM_Win // on windows we show a message box for errors
	//if error, break
	if (level == LogLevel::Error)
	{
		MessageBox(0, msg.c_str(), "ERROR", 0);
		abort();
	}
#endif // PLATFORM_Win

#endif // ndef SHIPPING 

	CheckBreak(level);
}

void Logger::CheckBreak(LogLevel level)
{
#if _DEBUG

#ifdef PLATFORM_x32
	if ((m_BreakBitField&level) == level)
	{
		__asm { int 3 };
	}
#else // PLATFORM_x64
	if ((m_BreakBitField&level) == level)
	{
		__debugbreak();
	}
#endif // PLATFORM_x32

#else // not debug
	if ((m_BreakBitField&level) == level) 
	{
		exit(-1);
	}

#endif // _DEBUG
}



//Console stuff
//***************
Logger::ConsoleLogger::ConsoleLogger()
{
	if (!AllocConsole())
	{
		std::cout << "Warning: Could not attach to console" << std::endl;
		CheckBreak(Error);
		return;
	}

	// Redirect the CRT standard input, output, and error handles to the console
	FILE* pCout;
	freopen_s(&pCout, "CONIN$", "r", stdin);
	freopen_s(&pCout, "CONOUT$", "w", stdout);
	freopen_s(&pCout, "CONOUT$", "w", stderr);

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

	m_os = &std::cout;
#ifdef PLATFORM_Win
	m_ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

void Logger::ConsoleLogger::SetColor(ConsoleLogger::Color color)
{
#ifdef PLATFORM_Win
	switch (color)
	{
	case Logger::ConsoleLogger::Color::WHITE:
		SetConsoleTextAttribute(m_ConsoleHandle, 15);
		break;
	case Logger::ConsoleLogger::Color::RED:
		SetConsoleTextAttribute(m_ConsoleHandle, 12);
		break;
	case Logger::ConsoleLogger::Color::GREEN:
		SetConsoleTextAttribute(m_ConsoleHandle, 10);
		break;
	case Logger::ConsoleLogger::Color::YELLOW:
		SetConsoleTextAttribute(m_ConsoleHandle, 14);
		break;
	case Logger::ConsoleLogger::Color::MAGENTA:
		SetConsoleTextAttribute(m_ConsoleHandle, 13);
		break;
	}
#endif
}

void Logger::ConsoleLogger::SetCursorPosition(ivec2 cursorPos)
{
#ifdef PLATFORM_Win
	COORD pos;
	pos.X = static_cast<int16>(cursorPos.x);
	pos.Y = static_cast<int16>(cursorPos.y);

	SetConsoleCursorPosition(m_ConsoleHandle, pos);
#endif
}

ivec2 Logger::ConsoleLogger::GetCursorPosition()
{
#ifdef PLATFORM_Win
	CONSOLE_SCREEN_BUFFER_INFO bufferInfo;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &bufferInfo))
	{
		ivec2 ret;
		ret.x = static_cast<int32>(bufferInfo.dwCursorPosition.X);
		ret.y = static_cast<int32>(bufferInfo.dwCursorPosition.Y);

		return ret;
	}
	else
	{
		DisplayError(TEXT("GetConsoleScreenBufferInfo"));
	}
#endif
	return ivec2(-1);
}

void Logger::DebugLogger::Log(const std::string& message)
{
#ifdef PLATFORM_Win
	OutputDebugString(message.c_str());
#endif
}
