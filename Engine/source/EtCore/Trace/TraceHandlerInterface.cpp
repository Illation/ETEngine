#include "stdafx.h"
#include "TraceHandlerInterface.h"

#include "TraceService.h"


namespace et {
namespace core {


//=========================
// Trace Handler Interface
//=========================


//-----------------------------------------
// I_TraceHandler::BuildBasicMessage
//
// Utility function to generate a trace message string
//
std::string I_TraceHandler::BuildBasicMessage(T_TraceContext const context,
	E_TraceLevel const level,
	std::string const& timestamp,
	std::string const& message)
{
	std::stringstream stream;

	stream << timestamp;

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
		ET_ERROR("Unhandled trace level");
		break;
	}

	stream << "[" << TraceService::GetContextContainer().GetContextName(context) << "] ";

	stream << message;

	stream << "\n";

	return stream.str();
}


} // namespace core
} // namespace et
