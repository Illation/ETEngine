#include "stdafx.h"
#include "RmlSystemInterface.h"


namespace et {
namespace gui {


//======================
// RML System Interface
//======================


//------------------------------------
// RmlSystemInterface::GetElapsedTime
//
// Seconds since start of the program
//
double RmlSystemInterface::GetElapsedTime()
{
	return core::ContextManager::GetInstance()->GetActiveContext()->time->GetTime<double>();
}

//---------------------------------
// RmlSystemInterface::LogMessage
//
bool RmlSystemInterface::LogMessage(Rml::Log::Type type, const Rml::String& message)
{
	core::LogLevel level = core::LogLevel::Info;
	switch (type)
	{
	case Rml::Log::Type::LT_ALWAYS:
	case Rml::Log::Type::LT_INFO:
	default:
		level = core::LogLevel::Info;
		break;

	case Rml::Log::Type::LT_WARNING:
		level = core::LogLevel::Warning;
		break;

	case Rml::Log::Type::LT_ERROR:
		level = core::LogLevel::Error;
		break;

	case Rml::Log::Type::LT_ASSERT:
		level = core::LogLevel::FixMe;
		break;

	case Rml::Log::Type::LT_DEBUG:
		level = core::LogLevel::Verbose;
		break;
	}


	core::Logger::Log(message.c_str(), level);

	return true;
}


} // namespace gui
} // namespace et
