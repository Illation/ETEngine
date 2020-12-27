#include <EtCore/stdafx.h>

#include <EtCooker/Cooker.h>

#include <Pipeline/linkerHelper.h>


//---------------------------------
// main
//
// Main function for the resource cooker - a simple command line program that compiles resources into a game ready format
//
int main(int argc, char *argv[])
{
	et::demo::ForceLinkingPipeline();

	et::cooker::RunCooker(argc, argv);
}
