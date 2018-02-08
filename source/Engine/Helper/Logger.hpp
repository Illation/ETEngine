#pragma once

enum LogLevel
{
	Info = 0x1,
	Warning = 0x2,
	Error = 0x4,
	FixMe = 0x8, 
	Verbose = 0x10
};

class Logger
{
public:
	static void Log(const string& msg, LogLevel level = LogLevel::Info,
		bool timestamp = false, ivec2 cursorPos = ivec2(-1));
	static void APIENTRY LogFormat(GLenum source, GLenum type, GLuint id,
		GLenum severity, GLsizei length, const GLchar* message, 
		const void* userParam);

	static ivec2 GetCursorPosition();

	static void StartFileLogging(const string& filename);
	static void StopFileLogging();

	static void UseTimestampDate(bool val) { m_TimestampDate = val; }

private:
	friend class AbstractFramework;

	static void Initialize();
	static void InitializeDebugOutput();
	static void Release();

	static void CheckBreak(LogLevel level);

private:
	class AbstractLogger
	{
	protected:
		ostream* m_os;
	public:
		AbstractLogger() {}
		virtual ~AbstractLogger() {}

		virtual void Log(const string& message)
		{
			(*m_os) << message;
			m_os->flush();
		}
		virtual void SetCursorPosition(ivec2 cursorPos) { UNUSED(cursorPos); }
	};

	class FileLogger : public AbstractLogger
	{
		string m_filename;
	public:
		explicit FileLogger(const string& fileName)
			: m_filename(fileName)
		{
			m_os = new ofstream(m_filename.c_str());
		}
		~FileLogger()
		{
			if (m_os)
			{
				ofstream* of = static_cast<ofstream*>(m_os);
				of->close();
				delete m_os;
			}
		}
	};

	class ConsoleLogger : public AbstractLogger
	{
	public:
		ConsoleLogger();
		enum class Color
		{
			WHITE,
			RED,
			GREEN,
			YELLOW,
			MAGENTA
		};
		void SetColor(ConsoleLogger::Color color);
		void SetCursorPosition(ivec2 cursorPos) override;
		ivec2 GetCursorPosition();
	private:
#ifdef PLATFORM_Win
		HANDLE m_ConsoleHandle;
#endif
	};

	class DebugLogger : public AbstractLogger
	{
	public:
		DebugLogger() {}
		virtual ~DebugLogger() {}
		void Log(const string& message)override;
	};

	static ConsoleLogger* m_ConsoleLogger;
	static FileLogger* m_FileLogger;
	static DebugLogger* m_DebugLogger;

	static uint8 m_BreakBitField;
	static bool m_TimestampDate;

private:
	//Disable default constructor and destructor
	Logger();
	~Logger();
	Logger(const Logger &obj);
	Logger& operator=(const Logger& obj);
};

