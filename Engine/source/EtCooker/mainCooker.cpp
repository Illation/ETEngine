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

#include <EtFramework/linkerHelper.h>


//============
// E T Cooker
//============


// forward declarations
void AddPackageToWriter(T_Hash const packageId, std::string const& dbBase, PackageWriter &writer, AssetDatabase& db);
void CookCompiledPackage(std::string const& dbBase, 
	std::string const& outPath, 
	std::string const& resName, 
	AssetDatabase& db, 
	std::string const& engineDbBase, 
	AssetDatabase& engineDb);
void CookFilePackages(std::string const& dbBase, 
	std::string const& outPath, 
	AssetDatabase& db,
	std::string const& engineDbBase,
	AssetDatabase& engineDb);


//---------------------------------
// main
//
// Main function for the resource cooker - a simple command line program that compiles resources into a game ready format
//
int main(int argc, char *argv[])
{
	et::fw::ForceLinking();

	// parse arguments
	//-----------------
	if (argc < 4)
	{
		std::cerr << "main > Not enough arguments, exiting! Usage: EtCooker.exe <database path> <out path> <create compiled resource [y/n]>" << std::endl;
		return 1;
	}
	FileUtil::SetExecutablePath(argv[0]); // working dir from executable path
	std::string databasePath(argv[1]);
	std::string engineDbPath(argv[2]);
	std::string outPath(argv[3]);
	bool const genCompiledResource = (std::string(argv[4]) == "y");
	ET_ASSERT(genCompiledResource || (std::string(argv[4]) == "n"), "Expected argument 4 to be either 'y' or 'n'!");

	// Init stuff
	//------------
	Logger::Initialize();
	Logger::StartFileLogging("cooker.log");

	LOG(FS("E.T.Cooker"));
	LOG(FS("//////////"));
	LOG("");
	LOG(FS(" - version: %s", et::build::Version::s_Name.c_str()));
	LOG("");

	AssetDatabase database;
	if (!serialization::DeserializeFromFile(databasePath, database))
	{
		LOG("main > unable to deserialize asset database at '" + std::string(databasePath) + std::string("'"), LogLevel::Error);
	}
	std::string dbBase = FileUtil::ExtractPath(databasePath);

	AssetDatabase engineDb;
	if (!serialization::DeserializeFromFile(engineDbPath, engineDb))
	{
		LOG("main > unable to deserialize engine database at '" + std::string(engineDbPath) + std::string("'"), LogLevel::Error);
	}
	std::string engineDbBase = FileUtil::ExtractPath(engineDbPath);

	if (genCompiledResource)
	{
		if (argc < 5)
		{
			std::cerr << "main > When specifying compiled resource, also specify the resource name in the last arg" << std::endl;
			return 2;
		}
		std::string resName(argv[5]);

		CookCompiledPackage(dbBase, outPath, resName, database, engineDbBase, engineDb);
	}
	else
	{
		CookFilePackages(dbBase, outPath, database, engineDbBase, engineDb);
	}

	// Clean up
	//----------
	Logger::Release();

	return 0;
}


////////////////////////////////////////////////////


//--------------------
// AddPackageToWriter
//
// Gets all assets in a package of that database and adds them to the package writer
//
void AddPackageToWriter(T_Hash const packageId, std::string const& dbBase, PackageWriter &writer, AssetDatabase& db)
{
	// Loop over files - add them to the writer
	AssetDatabase::T_AssetList assets = db.GetAssetsInPackage(packageId);
	for (I_Asset const* const asset : assets)
	{
		std::string const filePath = dbBase + asset->GetPath();
		std::string const assetName = asset->GetName();
		T_Hash const id = asset->GetId();

		LOG(assetName + std::string(" [") + std::to_string(id) + std::string("] @: ") + FileUtil::GetAbsolutePath(filePath));

		File* assetFile = new File(filePath + assetName, nullptr);
		writer.AddFile(assetFile, dbBase, E_CompressionType::Store);
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
	AssetDatabase& db,
	std::string const& engineDbBase,
	AssetDatabase& engineDb)
{
	// Create a package writer - all file paths will be written relative to our database directory
	PackageWriter packageWriter;
	std::vector<uint8> packageData;

	// serialize the asset database to a temporary file
	static std::string const s_TempPath("temp");
	std::string const dbName(ResourceManager::s_DatabasePath);
	std::string const tempDbFullPath = s_TempPath + std::string("/") + dbName;

	// Ensure the generated file directory exists
	Directory* tempDir = new Directory(s_TempPath, nullptr, true);

	AssetDatabase mergeDb(false);
	mergeDb.Merge(db);
	mergeDb.Merge(engineDb);
	if (!serialization::SerializeToFile(tempDbFullPath, mergeDb))
	{
		LOG(FS("CookCompiledPackage > Failed to serialize asset database to '%s'", tempDbFullPath.c_str()), LogLevel::Error);
	}

	// load the asset database from the temporary file and add it to the package
	File* dbFile = new File(dbName, tempDir);
	packageWriter.AddFile(dbFile, dbFile->GetPath(), E_CompressionType::Store);

	// add all other compiled files to the package
	static T_Hash const s_CompiledPackageId = 0u;
	AddPackageToWriter(s_CompiledPackageId, dbBase, packageWriter, db);
	AddPackageToWriter(s_CompiledPackageId, engineDbBase, packageWriter, engineDb);

	// write our package
	packageWriter.Write(packageData);

	// Generate source file
	GenerateCompilableResource(packageData, resName, outPath);

	// cleanup
	packageWriter.RemoveFile(dbFile);
	if (!tempDir->Delete())
	{
		LOG("CookCompiledPackage > Failed to clean up temporary file directory!", LogLevel::Error);
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
	AssetDatabase& db,
	std::string const& engineDbBase,
	AssetDatabase& engineDb)
{
	// Get a unified list of package descriptors
	std::vector<AssetDatabase::PackageDescriptor> descriptors = db.packages;
	for (AssetDatabase::PackageDescriptor const& desc : engineDb.packages)
	{
		// check if there is already a package with the same ID tracked in "descriptors"
		if (std::find_if(descriptors.cbegin(), descriptors.cend(), [&desc](AssetDatabase::PackageDescriptor const& tracked)
			{
				return tracked.GetId() == desc.GetId();
			}) == descriptors.cend())
		{
			descriptors.emplace_back(desc);
		}
	}

	// each package can have a separate asset list
	for (AssetDatabase::PackageDescriptor const& desc : descriptors)
	{
		PackageWriter packageWriter;
		std::vector<uint8> packageData;

		AddPackageToWriter(desc.GetId(), dbBase, packageWriter, db);
		AddPackageToWriter(desc.GetId(), engineDbBase, packageWriter, engineDb);

		// write our package
		packageWriter.Write(packageData);

		// Ensure the generated file directory exists
		Directory* dir = new Directory(outPath + desc.GetPath(), nullptr, true);

		// Create the output package file
		File* outFile = new File(desc.GetName() + FilePackage::s_PackageFileExtension, dir);
		FILE_ACCESS_FLAGS outFlags;
		outFlags.SetFlags(FILE_ACCESS_FLAGS::FLAGS::Create | FILE_ACCESS_FLAGS::FLAGS::Exists);
		if (!outFile->Open(FILE_ACCESS_MODE::Write, outFlags))
		{
			LOG("CookFilePackages > Failed to open file " + outFile->GetName(), LogLevel::Warning);
			continue;
		}

		// Write the package data
		outFile->Write(packageData);

		// cleanup
		SafeDelete(outFile);
		SafeDelete(dir);
	}
}

