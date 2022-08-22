#include <EtGUI/stdafx.h>

#include <EtGUI/linkerHelper.h>

#include <EtPipeline/Core/linkerHelper.h>
#include <EtPipeline/RHI/linkerHelper.h>
#include <EtPipeline/GUI/linkerHelper.h>

#include <EtCooker/Core/Cooker.h>



//---------------------------------
// main
//
// Main function for the resource cooker - a simple command line program that compiles resources into a game ready format
//
int main(int argc, char *argv[])
{
	et::gui::ForceLinking();
	et::pl::ForceLinking();
	et::pl::ForceLinking_RHI();
	et::pl::ForceLinking_GUI();

	et::cooker::Cooker cooker(argc, argv);
	if (cooker.GetReturnCode() == et::cooker::Cooker::E_ReturnCode::Success)
	{
		cooker.Run();
	}

	return static_cast<int>(cooker.GetReturnCode());
}
