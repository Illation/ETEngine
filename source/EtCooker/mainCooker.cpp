#include <EtCore/stdafx.h>

#include "linkerHelper.h"

#include <EtCore/Helper/Logger.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/Content/ResourceManager.h>


//---------------------------------
// main
//
// Main function for the resource cooker
//
int main(int argc, char* argv[]) 
{
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

	// Loop over files
	ResourceManager::AssetDatabase& db = ResourceManager::GetInstance()->GetDatabase();
	for (ResourceManager::AssetDatabase::AssetCache& cache : db.caches)
	{
		for (I_Asset* asset : cache.cache)
		{
			std::string const filePath = asset->GetPath();
			std::string const assetName = asset->GetName();
			T_Hash const id = asset->GetId();

			LOG(assetName + std::string(" [") + std::to_string(id) + std::string("] @: ") + filePath);
		}
	}

	// Clean up
	ResourceManager::GetInstance()->DestroyInstance();

	Logger::Release();

	return 0;
}