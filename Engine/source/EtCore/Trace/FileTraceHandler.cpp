#include "stdafx.h"
#include "FileTraceHandler.h"


namespace et {
namespace core {


//====================
// File Trace Handler
//====================


//----------------------------
// FileTraceHandler::d-tor
//
// Close the file
//
FileTraceHandler::~FileTraceHandler()
{
	m_FileStream->close();
}

//---------------------------------
// FileTraceHandler::Initialize
//
// Open a file to output text to
//
bool FileTraceHandler::Initialize()
{
	m_FileStream = Create<std::ofstream>(m_FileName.c_str());
	return m_FileStream->is_open();
}

//-------------------------------------
// FileTraceHandler::OnTraceMessage
//
void FileTraceHandler::OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message)
{
	*m_FileStream << I_TraceHandler::BuildBasicMessage(context, level, timestamp, message);
	m_FileStream->flush();
}


} // namespace core
} // namespace et

