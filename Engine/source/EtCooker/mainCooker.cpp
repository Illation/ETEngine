#include <EtCore/stdafx.h>

#include "PackageWriter.h"
#include "CompiledDataGenerator.h"

#include <EtCore/Helper/Logger.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/Package/FilePackage.h>
#include <EtCore/Reflection/Serialization.h>
#include <EtCore/Content/AssetDatabase.h>
#include <EtCore/Content/ResourceManager.h>

#include <Engine/linkerHelper.h>


//============
// E T Cooker
//============


// forward declarations
void AddPackageToWriter(T_Hash const packageId, std::string const& dbBase, PackageWriter &writer, AssetDatabase& db);
void CookCompiledPackage(std::string const& dbBase, std::string const& outPath, std::string const& resName, AssetDatabase& db);
void CookFilePackages(std::string const& dbBase, std::string const& outPath, AssetDatabase& db);


//---------------------------------
// main
//
// Main function for the resource cooker - a simple command line program that compiles resources into a game ready format
//
int main(int argc, char *argv[])
{
	ForceLinking();

	// parse arguments
	//-----------------
	if (argc < 4)
	{
		std::cerr << "main > Not enough arguments, exiting! Usage: EtCooker.exe <database path> <out path> <create compiled resource [y/n]>" << std::endl;
		return 1;
	}
	FileUtil::SetExecutablePath(argv[0]); // working dir from executable path
	std::string databasePath(argv[1]);
	std::string outPath(argv[2]);
	bool const genCompiledResource = (std::string(argv[3]) == "y");
	ET_ASSERT(genCompiledResource || (std::string(argv[3]) == "n"), "Expected argument 3 to be either 'y' or 'n'!");

	// Init stuff
	//------------
	Logger::Initialize();
	Logger::StartFileLogging("cooker.log");

	AssetDatabase database;
	if (!serialization::DeserializeFromFile(databasePath, database))
	{
		LOG("main > unable to deserialize asset database at '" + std::string(databasePath) + std::string("'"), LogLevel::Error);
	}
	std::string dbBase = FileUtil::ExtractPath(databasePath);

	if (genCompiledResource)
	{
		if (argc < 5)
		{
			std::cerr << "main > When specifying compiled resource, also specify the resource name in the last arg" << std::endl;
			return 2;
		}
		std::string resName(argv[4]);

		CookCompiledPackage(dbBase, outPath, resName, database);
	}
	else
	{
		CookFilePackages(dbBase, outPath, database);
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
void CookCompiledPackage(std::string const& dbBase, std::string const& outPath, std::string const& resName, AssetDatabase& db)
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

	if (!serialization::SerializeToFile(tempDbFullPath, db))
	{
		LOG(FS("CookCompiledPackage > Failed to serialize asset database to '%s'", tempDbFullPath.c_str()), LogLevel::Error);
	}

	// load the asset database from the temporary file and add it to the package
	File* dbFile = new File(dbName, tempDir);
	packageWriter.AddFile(dbFile, dbFile->GetPath(), E_CompressionType::Store);

	// add all other compiled files to the package
	static T_Hash const s_CompiledPackageId = 0u;
	AddPackageToWriter(s_CompiledPackageId, dbBase, packageWriter, db);

	// write our package
	packageWriter.Write(packageData);

	// Generate source file
	GenerateCompilableResource(packageData, resName, outPath);

	// cleanup
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
void CookFilePackages(std::string const& dbBase, std::string const& outPath, AssetDatabase& db)
{
	// each package can have a separate asset list
	for (AssetDatabase::PackageDescriptor const& desc : db.packages)
	{
		PackageWriter packageWriter;
		std::vector<uint8> packageData;

		AddPackageToWriter(desc.GetId(), dbBase, packageWriter, db);

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

