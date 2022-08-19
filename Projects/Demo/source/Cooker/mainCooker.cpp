#include <EtApplication/stdafx.h>

#include <EtCore/FileSystem/Entry.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtFramework/Config/BootConfig.h>

#include <EtPipeline/Core/Content/FileResourceManager.h>

#include <Pipeline/linkerHelper.h>

#include <EtCooker/Core/Cooker.h>



//---------------------------------
// main
//
// Main function for the resource cooker - a simple command line program that compiles resources into a game ready format
//
int main(int argc, char *argv[])
{
	using namespace et;
	using namespace et::cooker;

	demo::ForceLinkingPipeline();

	Cooker cooker(argc, argv);
	if (cooker.GetReturnCode() == Cooker::E_ReturnCode::Success)
	{
		// needed for some asset conversions
		render::RenderingSystems::AddReference();

		// add the boot config
		cooker.RegisterPreWritePackageFn(Cooker::T_PreWritePackageFn(
			[&cooker](et::core::PackageDescriptor const* const desc, PackageWriter& writer)
			{
				if (desc == nullptr) // compiled package
				{
					core::File* cfgFile = new core::File(cooker.GetProjectResourcePath() + fw::BootConfig::s_FileName, nullptr);
					writer.AddFile(cfgFile, cooker.GetProjectResourcePath(), core::E_CompressionType::Store);
				}
			}));

		cooker.Run();

		et::render::RenderingSystems::RemoveReference();
	}

	return static_cast<int>(cooker.GetReturnCode());
}
