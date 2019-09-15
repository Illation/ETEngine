#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include <vector>
#include <iostream>
#include <string>

#include "mainTesting.h"

#include <EtCore/Helper/AtomicTypes.h>
#include <EtCore/FileSystem/FileUtil.h>


std::string global::g_UnitTestDir = std::string();


int main(int argc, char* argv[]) 
{
	// working dir
	if (argc > 0)
	{
		FileUtil::SetExecutablePath(argv[0]);
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

	int result = Catch::Session().run(argc, argv);

	return result;
}