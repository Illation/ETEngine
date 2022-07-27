#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <vector>
#include <iostream>
#include <string>

#include "mainTesting.h"

#include <EtCore/Util/AtomicTypes.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Trace/TraceService.h>


std::string global::g_UnitTestDir = std::string();


int main(int argc, char* argv[]) 
{
	// working dir
	if (argc > 0)
	{
		et::core::FileUtil::SetExecutablePath(argv[0]);
	}
	else
	{
		std::cerr << "main > Couldn't extract working directory from arguments, exiting!" << std::endl;
		return 1;
	}

	// root directory for file related tests
	if (argc > 1)
	{
		global::g_UnitTestDir = std::string(argv[1]);
		argc = 1;
	}
	else
	{
		std::cerr << "main > Couldn't extract test directory from arguments, exiting!" << std::endl;
		return 2;
	}

	Catch::Session session;

	et::core::TraceService::Initialize();

	int result = session.run(argc, argv);

	et::core::TraceService::Destroy();

	return result;
}