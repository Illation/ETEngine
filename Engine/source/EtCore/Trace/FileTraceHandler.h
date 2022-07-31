#pragma once
#include "TraceHandlerInterface.h"


namespace et {
namespace core {


//------------------
// FileTraceHandler
//
// Processes trace messages by writing them to a file
//
class FileTraceHandler : public I_TraceHandler
{
	RTTR_ENABLE(I_TraceHandler)

public:

	FileTraceHandler(std::string const& fileName) : m_FileName(fileName) {}
	virtual ~FileTraceHandler();

	bool Initialize() override;
	virtual void OnTraceMessage(T_TraceContext const context, E_TraceLevel const level, std::string const& timestamp, std::string const& message) override;


	// Data
	///////

private:
	std::string m_FileName;
	UniquePtr<std::ofstream> m_FileStream; // #todo: after file system rewrite, use engine file system
};


} // namespace core
} // namespace et
