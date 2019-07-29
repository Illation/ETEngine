#include <EtCore/stdafx.h>

#include "linkerHelper.h"
#include "CompiledDataGenerator.h"

#include <EtCore/Helper/Logger.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/Package/PackageWriter.h>
#include <EtCore/Content/ResourceManager.h>


//---------------------------------
// main
//
// Main function for the resource cooker
//
int main(int argc, char *argv[])
{
	ForceLinking();

	// working dir
	if (argc < 1)
	{
		std::cerr << "main > Couldn't extract working directory from arguments, exiting!" << std::endl;
		return 1;
	}
	FileUtil::SetExecutablePath(argv[0]);

	Logger::Initialize();

	// Init database
	if (argc < 2)
	{
		// not error so that we can return with an error code
		LOG("main > Couldn't get asset_database path from arguments, exiting!", LogLevel::Warning);
		return 2;
	}
	std::string databasePath(argv[1]);
	ResourceManager::GetInstance()->InitFromFile(databasePath);
	std::string dbBase = FileUtil::ExtractPath(databasePath);

	// Create a package writer
	PackageWriter packageWriter;

	// add the asset database to the compiled package
	File* dbFile = new File(databasePath, nullptr);
	packageWriter.AddFile(dbFile, E_CompressionType::Store);

	// Loop over files - add them to the writer
	ResourceManager::AssetDatabase& db = ResourceManager::GetInstance()->GetDatabase();
	for (ResourceManager::AssetDatabase::AssetCache& cache : db.caches)
	{
		for (I_Asset* asset : cache.cache)
		{
			std::string const filePath = dbBase + asset->GetPath();
			std::string const assetName = asset->GetName();
			T_Hash const id = asset->GetId();

			LOG(assetName + std::string(" [") + std::to_string(id) + std::string("] @: ") + FileUtil::GetAbsolutePath(filePath));

			File* assetFile = new File(filePath + assetName, nullptr);
			packageWriter.AddFile(assetFile, E_CompressionType::Store);
		}
	}

	// write our package
	std::vector<uint8> packageData;
	packageWriter.Write(packageData);

	// Generate source file
	if (argc < 3)
	{
		LOG("main > Couldn't get generated file path from arguments, exiting!", LogLevel::Warning);
		return 3;
	}
	std::string generatedPath(argv[2]);

	GenerateCompilableResource(packageData, "compiled_package", generatedPath);

	// Clean up
	ResourceManager::GetInstance()->DestroyInstance();

	Logger::Release();

	return 0;
}