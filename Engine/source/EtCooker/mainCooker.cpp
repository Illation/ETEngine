#include <EtCore/stdafx.h>

#include "PackageWriter.h"
#include "CompiledDataGenerator.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/Util/Logger.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/Package/FilePackage.h>
#include <EtCore/Reflection/Serialization.h>
#include <EtCore/Content/AssetDatabase.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtFramework/Config/BootConfig.h>
#include <EtFramework/linkerHelper.h>

#include <EtPipeline/linkerHelper.h>
#include <EtPipeline/Content/EditorAssetDatabase.h>


//============
// E T Cooker
//============


namespace et { namespace cooker {
	int RunCooker(int argc, char *argv[]);
} }


//---------------------------------
// main
//
// Main function for the resource cooker - a simple command line program that compiles resources into a game ready format
//
int main(int argc, char *argv[])
{
	et::cooker::RunCooker(argc, argv);
}


namespace et {
namespace cooker {


// forward declarations
void AddPackageToWriter(core::HashString const packageId, 
	std::string const& dbBase, 
	PackageWriter &writer, 
	pl::EditorAssetDatabase& db, 
	bool const setupRuntime);
void CookCompiledPackage(std::string const& dbBase, 
	std::string const& outPath, 
	std::string const& resName, 
	pl::EditorAssetDatabase& db, 
	std::string const& engineDbBase, 
	pl::EditorAssetDatabase& engineDb);
void CookFilePackages(std::string const& dbBase, 
	std::string const& outPath, 
	pl::EditorAssetDatabase& db,
	std::string const& engineDbBase,
	pl::EditorAssetDatabase& engineDb);


////////////////////////////////////////////////////


int RunCooker(int argc, char *argv[])
{
	fw::ForceLinking();
	pl::ForceLinking();

	// parse arguments
	//-----------------
	if (argc < 4)
	{
		std::cerr << "main > Not enough arguments, exiting! Usage: EtCooker.exe <database path> <out path> <create compiled resource [y/n]>" << std::endl;
		return 1;
	}
	core::FileUtil::SetExecutablePath(argv[0]); // working dir from executable path
	std::string databasePath(argv[1]);
	std::string engineDbPath(argv[2]);
	std::string outPath(argv[3]);
	bool const genCompiledResource = (std::string(argv[4]) == "y");
	ET_ASSERT(genCompiledResource || (std::string(argv[4]) == "n"), "Expected argument 4 to be either 'y' or 'n'!");

	// Init stuff
	//------------
	core::Logger::Initialize();
	core::Logger::StartFileLogging("cooker.log");

	LOG(FS("E.T.Cooker"));
	LOG(FS("//////////"));
	LOG("");
	LOG(FS(" - version: %s", build::Version::s_Name.c_str()));
	LOG("");

	pl::EditorAssetDatabase database;
	pl::EditorAssetDatabase::InitDb(database, databasePath);
	std::string dbBase = core::FileUtil::ExtractPath(databasePath);

	pl::EditorAssetDatabase engineDb;
	pl::EditorAssetDatabase::InitDb(engineDb, engineDbPath);
	std::string engineDbBase = core::FileUtil::ExtractPath(engineDbPath);

	if (genCompiledResource)
	{
		if (argc < 5)
		{
			std::cerr << "main > When specifying compiled resource, also specify the resource name in the last arg" << std::endl;
			return 2;
		}

		std::string resName(argv[5]);

		database.SetupAllRuntimeAssets();
		engineDb.SetupAllRuntimeAssets();

		CookCompiledPackage(dbBase, outPath, resName, database, engineDbBase, engineDb);
	}
	else
	{
		CookFilePackages(dbBase, outPath, database, engineDbBase, engineDb);
	}

	// Clean up
	//----------
	core::Logger::Release();

	return 0;
}


//--------------------
// AddPackageToWriter
//
// Gets all assets in a package of that database and adds them to the package writer
//
void AddPackageToWriter(core::HashString const packageId, 
	std::string const& dbBase, 
	PackageWriter &writer, 
	pl::EditorAssetDatabase& db, 
	bool const setupRuntime)
{
	// Loop over files - add them to the writer
	pl::EditorAssetDatabase::T_AssetList assets = db.GetAssetsInPackage(packageId);
	std::string const baseAssetPath = dbBase + db.GetAssetPath();
	for (pl::EditorAssetBase* const editorAsset : assets)
	{
		if (setupRuntime)
		{
			editorAsset->SetupRuntimeAssets();
		}

		std::vector<core::I_Asset*> const runtimeAssets = editorAsset->GetAllRuntimeAssets();
		for (core::I_Asset const* const asset : runtimeAssets)
		{
			std::string const filePath = baseAssetPath + asset->GetPath();
			std::string const assetName = asset->GetName();
			core::HashString const id = asset->GetId();

			LOG(assetName + std::string(" [") + std::to_string(id.Get()) + std::string("] @: ") + core::FileUtil::GetAbsolutePath(filePath));

			core::File* const assetFile = new core::File(filePath + assetName, nullptr);
			writer.AddFile(assetFile, baseAssetPath, core::E_CompressionType::Store);
		}
	}
}

//---------------------
// CookCompiledPackage
//
// Writes the package with compiled data that ends up as a generated source file.
//  - this includes the file for the asset database
//
void CookCompiledPackage(std::string const& dbBase, 
	std::string const& outPath, 
	std::string const& resName, 
	pl::EditorAssetDatabase& db,
	std::string const& engineDbBase,
	pl::EditorAssetDatabase& engineDb)
{
	// Create a package writer - all file paths will be written relative to our database directory
	PackageWriter packageWriter;
	std::vector<uint8> packageData;

	// serialize the asset database to a temporary file
	static std::string const s_TempPath("temp");
	std::string const dbName(core::ResourceManager::s_DatabasePath);
	std::string const tempDbFullPath = s_TempPath + std::string("/") + dbName;

	// Ensure the generated file directory exists
	core::Directory* tempDir = new core::Directory(s_TempPath, nullptr, true);

	core::AssetDatabase mergeDb(false);
	db.PopulateAssetDatabase(mergeDb);
	engineDb.PopulateAssetDatabase(mergeDb);
	if (!core::serialization::SerializeToFile(tempDbFullPath, mergeDb))
	{
		LOG(FS("CookCompiledPackage > Failed to serialize asset database to '%s'", tempDbFullPath.c_str()), core::LogLevel::Error);
	}

	// load the asset database from the temporary file and add it to the package
	core::File* dbFile = new core::File(dbName, tempDir);
	packageWriter.AddFile(dbFile, dbFile->GetPath(), core::E_CompressionType::Store);

	// add the boot config
	core::File* cfgFile = new core::File(dbBase + fw::BootConfig::s_FileName, nullptr);
	packageWriter.AddFile(cfgFile, dbBase, core::E_CompressionType::Store);

	// add all other compiled files to the package
	static core::HashString const s_CompiledPackageId;
	AddPackageToWriter(s_CompiledPackageId, dbBase, packageWriter, db, false);
	AddPackageToWriter(s_CompiledPackageId, engineDbBase, packageWriter, engineDb, false);

	// write our package
	packageWriter.Write(packageData);

	// Generate source file
	GenerateCompilableResource(packageData, resName, outPath);

	// cleanup
	packageWriter.RemoveFile(dbFile);
	if (!tempDir->Delete())
	{
		LOG("CookCompiledPackage > Failed to clean up temporary file directory!", core::LogLevel::Error);
		delete tempDir;
	}
	tempDir = nullptr;
}

//---------------------
// CookCompiledPackage
//
// Writes the package with compiled data that ends up as a generated source file.
//  - this includes the file for the asset database
//
void CookFilePackages(std::string const& dbBase, 
	std::string const& outPath, 
	pl::EditorAssetDatabase& db,
	std::string const& engineDbBase,
	pl::EditorAssetDatabase& engineDb)
{
	// Get a unified list of package descriptors
	std::vector<core::PackageDescriptor> descriptors = db.GetPackages();
	for (core::PackageDescriptor const& desc : engineDb.GetPackages())
	{
		// check if there is already a package with the same ID tracked in "descriptors"
		if (std::find_if(descriptors.cbegin(), descriptors.cend(), [&desc](core::PackageDescriptor const& tracked)
			{
				return tracked.GetId() == desc.GetId();
			}) == descriptors.cend())
		{
			descriptors.emplace_back(desc);
		}
	}

	// each package can have a separate asset list
	for (core::PackageDescriptor const& desc : descriptors)
	{
		if (!desc.IsRuntime())
		{
			continue;
		}

		PackageWriter packageWriter;
		std::vector<uint8> packageData;

		AddPackageToWriter(desc.GetId(), dbBase, packageWriter, db, true);
		AddPackageToWriter(desc.GetId(), engineDbBase, packageWriter, engineDb, true);

		// write our package
		packageWriter.Write(packageData);

		// Ensure the generated file directory exists
		core::Directory* dir = new core::Directory(outPath + desc.GetPath(), nullptr, true);

		// Create the output package file
		core::File* outFile = new core::File(desc.GetName() + core::FilePackage::s_PackageFileExtension, dir);
		core::FILE_ACCESS_FLAGS outFlags;
		outFlags.SetFlags(core::FILE_ACCESS_FLAGS::FLAGS::Create | core::FILE_ACCESS_FLAGS::FLAGS::Exists);
		if (!outFile->Open(core::FILE_ACCESS_MODE::Write, outFlags))
		{
			LOG("CookFilePackages > Failed to open file " + outFile->GetName(), core::LogLevel::Warning);
			continue;
		}

		// Write the package data
		outFile->Write(packageData);

		// cleanup
		SafeDelete(outFile);
		SafeDelete(dir);
	}
}


} // namespace cooker
} // namespace et
