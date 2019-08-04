#include <EtCore/stdafx.h>

#include "PackageWriter.h"
#include "CompiledDataGenerator.h"

#include <EtCore/Helper/Logger.h>
#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/Package/FilePackage.h>
#include <EtCore/Content/ResourceManager.h>

#include <Engine/linkerHelper.h>


//============
// E T Cooker
//============


// forward declarations
void WritePackageToData(T_Hash const packageId, std::string const& dbBase, PackageWriter &packageWriter, std::vector<uint8>& packageData);
void CookCompiledPackage(std::string const& dbBase, std::string const& databasePath, std::string const& outPath, std::string const& resName);
void CookFilePackages(std::string const& dbBase, std::string const& outPath);


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

	ResourceManager::GetInstance()->InitFromFile(databasePath);
	std::string dbBase = FileUtil::ExtractPath(databasePath);

	if (genCompiledResource)
	{
		if (argc < 5)
		{
			std::cerr << "main > When specifying compiled resource, also specify the resource name in the last arg" << std::endl;
			return 2;
		}
		std::string resName(argv[4]);

		CookCompiledPackage(dbBase, databasePath, outPath, resName);
	}
	else
	{
		CookFilePackages(dbBase, outPath);
	}

	// Clean up
	//----------
	ResourceManager::GetInstance()->DestroyInstance();

	Logger::Release();

	return 0;
}


////////////////////////////////////////////////////


//--------------------
// WritePackageToData
//
// Gets all assets in a package and creates a binary data blob for it
//
void WritePackageToData(T_Hash const packageId, std::string const& dbBase, PackageWriter &packageWriter, std::vector<uint8>& packageData)
{
	// Loop over files - add them to the writer
	AssetDatabase::T_AssetList assets = ResourceManager::GetInstance()->GetDatabase().GetAssetsInPackage(packageId);
	for (I_Asset const* const asset : assets)
	{
		std::string const filePath = dbBase + asset->GetPath();
		std::string const assetName = asset->GetName();
		T_Hash const id = asset->GetId();

		LOG(assetName + std::string(" [") + std::to_string(id) + std::string("] @: ") + FileUtil::GetAbsolutePath(filePath));

		File* assetFile = new File(filePath + assetName, nullptr);
		packageWriter.AddFile(assetFile, E_CompressionType::Store);
	}

	// write our package
	packageWriter.Write(packageData);
}

//---------------------
// CookCompiledPackage
//
// Writes the package with compiled data that ends up as a generated source file.
//  - this includes the file for the asset database
//
void CookCompiledPackage(std::string const& dbBase, std::string const& databasePath, std::string const& outPath, std::string const& resName)
{
	// Create a package writer - all file paths will be written relative to our database directory
	PackageWriter packageWriter(dbBase);
	std::vector<uint8> packageData;

	// add the asset database to the compiled package
	File* dbFile = new File(databasePath, nullptr);
	packageWriter.AddFile(dbFile, E_CompressionType::Store);

	static T_Hash const s_CompiledPackageId = 0u;
	WritePackageToData(s_CompiledPackageId, dbBase, packageWriter, packageData);

	// Generate source file
	GenerateCompilableResource(packageData, resName, outPath);
}

//---------------------
// CookCompiledPackage
//
// Writes the package with compiled data that ends up as a generated source file.
//  - this includes the file for the asset database
//
void CookFilePackages(std::string const& dbBase, std::string const& outPath)
{
	// each package can have a separate asset list
	for (AssetDatabase::PackageDescriptor const& desc : ResourceManager::GetInstance()->GetDatabase().packages)
	{
		PackageWriter packageWriter(dbBase);
		std::vector<uint8> packageData;

		WritePackageToData(desc.GetId(), dbBase, packageWriter, packageData);

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

