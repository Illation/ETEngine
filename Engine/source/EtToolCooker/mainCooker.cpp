#include <EtCore/stdafx.h>

#include <EtCooker/Cooker.h>



//---------------------------------
// main
//
// Main function for the resource cooker - a simple command line program that compiles resources into a game ready format
//
int main(int argc, char *argv[])
{
	et::cooker::Cooker cooker(argc, argv);
	if (cooker.GetReturnCode() == et::cooker::Cooker::E_ReturnCode::Success)
	{
		cooker.Run();
	}

	return static_cast<int>(cooker.GetReturnCode());
}
