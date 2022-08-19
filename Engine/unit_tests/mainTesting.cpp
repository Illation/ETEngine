#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <vector>
#include <iostream>
#include <string>

#include <EtCore/stdafx.h>

#include "mainTesting.h"

#include <EtCore/Trace/Assert.h>
#include <EtCore/Platform/AtomicTypes.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Reflection/ReflectionUtil.h>
#include <EtCore/Reflection/TypeInfoRegistry.h>
#include <EtCore/Trace/Trace.h>
#include <EtCore/Util/CommandLine.h>


std::string global::g_UnitTestDir = std::string();
ET_REGISTER_COMMANDLINE(test_dir, global::g_UnitTestDir, "directory for file system dependent tests");


int main(int argc, char* argv[]) 
{
	using namespace et;

	// working dir
	if (argc > 0)
	{
		core::FileUtil::SetExecutablePath(argv[0]);
	}
	else
	{
		std::cerr << "main > Couldn't extract working directory from arguments, exiting!" << std::endl;
		return -1;
	}

	core::TraceService::Initialize();
	core::TraceService::Instance()->SetupDefaultHandlers("ET unit tests", true); 

	core::TypeInfoRegistry::Instance().Initialize();

	core::CommandLineParser::Instance().Process(argc, argv);
	argc = 1;

	if (!core::CommandLineParser::Instance().WasOptionSet(global::g_UnitTestDir))
	{
		ET_ERROR("Unit test directory needs to be set for file based tests with command line option '--test_dir'!");
	}

	Catch::Session session;

	int result = session.run(argc, argv);

	core::TraceService::Destroy();

	return result;
}


// Override catch out streams so that they use the engine trace

namespace et {
	ET_REGISTER_TRACE_CTX(ET_CTX_TEST);
}

namespace et {
namespace testing {


//----------------
// TraceOutBuffer
//
// string buffer that sends the input to trace
//
class TraceOutBuffer final : public std::stringbuf 
{
public:
	TraceOutBuffer(core::E_TraceLevel const traceLevel) : m_TraceLevel(traceLevel) {}
    ~TraceOutBuffer() = default;

    int sync() override 
	{
        int ret = 0;
		std::string const message = str();
		ET_LOG(ET_CTX_TEST, m_TraceLevel, false, message.c_str());

        // Reset the buffer to avoid printing it multiple times
        str("");
        return ret;
    }

private:
	core::E_TraceLevel const m_TraceLevel;
};


} // namespace testing 
} // namespace et 


namespace Catch {

std::ostream& cout() 
{
	static std::ostream ret(new et::testing::TraceOutBuffer(et::core::E_TraceLevel::TL_Info));
	return ret;
}

std::ostream& clog() 
{
	static std::ostream ret(new et::testing::TraceOutBuffer(et::core::E_TraceLevel::TL_Warning));
	return ret;
}

std::ostream& cerr() 
{
	static std::ostream ret(new et::testing::TraceOutBuffer(et::core::E_TraceLevel::TL_Error));
	return ret;
}

} // namespace Catch