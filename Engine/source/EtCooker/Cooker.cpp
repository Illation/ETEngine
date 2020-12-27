#include <EtCore/stdafx.h>
#include "Cooker.h"

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


namespace et {
namespace cooker {


//============
// E T Cooker
//============


//---------------
// Cooker::c-tor
//
// Sets up according to commandline arguments
//
Cooker::Cooker(int32 const argc, char* const argv[])
{
	// parse arguments
	//-----------------
	if (argc < 4)
	{
		std::cerr 
			<< "Cooker::c-tor > Not enough arguments, exiting! Usage: EtCooker.exe <database path> <out path> <create compiled resource [y/n]>" 
			<< std::endl;
		m_ReturnCode = E_ReturnCode::InsufficientArguments;
		return;
	}

	core::FileUtil::SetExecutablePath(argv[0]); // working dir from executable path

	std::string databasePath(argv[1]);
	std::string engineDbPath(argv[2]);
	m_OutPath = argv[3];
	m_GenerateCompiled = (std::string(argv[4]) == "y");

	if (m_GenerateCompiled)
	{
		if (argc < 5)
		{
			std::cerr << "main > When specifying compiled resource, also specify the resource name in the last arg" << std::endl;
			m_ReturnCode = E_ReturnCode::MissingResourceName;
			return;
		}

		m_ResourceName = argv[5];
	}

	// Init stuff
	//------------
	core::Logger::Initialize();
	core::Logger::StartFileLogging("cooker.log");

	ET_ASSERT(m_GenerateCompiled || (std::string(argv[4]) == "n"), "Expected argument 4 to be either 'y' or 'n'!");

	LOG(FS("E.T.Cooker"));
	LOG(FS("//////////"));
	LOG("");
	LOG(FS(" - version: %s", build::Version::s_Name.c_str()));
	LOG("");

	pl::EditorAssetDatabase::InitDb(m_ProjectDb, databasePath);
	m_ProjectPath = core::FileUtil::ExtractPath(databasePath);

	pl::EditorAssetDatabase::InitDb(m_EngineDb, engineDbPath);
	m_EnginePath = core::FileUtil::ExtractPath(engineDbPath);
}

//---------------
// Cooker::d-tor
//
Cooker::~Cooker()
{
	core::Logger::Release();
}

//-------------
// Cooker::Run
//
// Run according to arguments passed to class
//
void Cooker::Run()
{
	if (m_GenerateCompiled)
	{
		m_ProjectDb.SetupAllRuntimeAssets();
		m_EngineDb.SetupAllRuntimeAssets();

		CookCompiledPackage();
	}
	else
	{
		CookFilePackages();
	}
}

//---------------------
// CookCompiledPackage
//
// Writes the package with compiled data that ends up as a generated source file.
//  - this includes the file for the asset database
//
void Cooker::CookCompiledPackage()
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
	m_ProjectDb.PopulateAssetDatabase(mergeDb);
	m_EngineDb.PopulateAssetDatabase(mergeDb);
	if (!core::serialization::SerializeToFile(tempDbFullPath, mergeDb))
	{
		LOG(FS("CookCompiledPackage > Failed to serialize asset database to '%s'", tempDbFullPath.c_str()), core::LogLevel::Error);
		m_ReturnCode = E_ReturnCode::FailedToSerialize;
		return;
	}

	// load the asset database from the temporary file and add it to the package
	core::File* dbFile = new core::File(dbName, tempDir);
	packageWriter.AddFile(dbFile, dbFile->GetPath(), core::E_CompressionType::Store);

	// add the boot config
	core::File* cfgFile = new core::File(m_ProjectPath + fw::BootConfig::s_FileName, nullptr);
	packageWriter.AddFile(cfgFile, m_ProjectPath, core::E_CompressionType::Store);

	// add all other compiled files to the package
	static core::HashString const s_CompiledPackageId;
	AddPackageToWriter(s_CompiledPackageId, m_ProjectPath, packageWriter, m_ProjectDb);
	AddPackageToWriter(s_CompiledPackageId, m_EnginePath, packageWriter, m_EngineDb);

	// write our package
	packageWriter.Write(packageData);

	// Generate source file
	GenerateCompilableResource(packageData, m_ResourceName, m_OutPath);

	// cleanup
	packageWriter.RemoveFile(dbFile);
	if (!tempDir->Delete())
	{
		LOG("CookCompiledPackage > Failed to clean up temporary file directory!", core::LogLevel::Error);
		delete tempDir;
		m_ReturnCode = E_ReturnCode::FailedToCleanup;
	}

	tempDir = nullptr;
}

//-----------------------------
// Cooker::CookCompiledPackage
//
// Writes all packages that don't get generated backed into the executable 
//
void Cooker::CookFilePackages()
{
	// Get a unified list of package descriptors
	std::vector<core::PackageDescriptor> descriptors = m_ProjectDb.GetPackages();
	for (core::PackageDescriptor const& desc : m_EngineDb.GetPackages())
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

		AddPackageToWriter(desc.GetId(), m_ProjectPath, packageWriter, m_ProjectDb);
		AddPackageToWriter(desc.GetId(), m_EnginePath, packageWriter, m_EngineDb);

		// write our package
		packageWriter.Write(packageData);

		// Ensure the generated file directory exists
		core::Directory* dir = new core::Directory(m_OutPath + desc.GetPath(), nullptr, true);

		// Create the output package file
		core::File* outFile = new core::File(desc.GetName() + core::FilePackage::s_PackageFileExtension, dir);
		core::FILE_ACCESS_FLAGS outFlags;
		outFlags.SetFlags(core::FILE_ACCESS_FLAGS::FLAGS::Create | core::FILE_ACCESS_FLAGS::FLAGS::Exists);
		if (!outFile->Open(core::FILE_ACCESS_MODE::Write, outFlags))
		{
			LOG("CookFilePackages > Failed to open file " + outFile->GetName(), core::LogLevel::Warning);
			m_ReturnCode = E_ReturnCode::FailedToWritePackage;
			continue;
		}

		// Write the package data
		outFile->Write(packageData);

		// cleanup
		SafeDelete(outFile);
		SafeDelete(dir);
	}
}

//----------------------------
// Cooker::AddPackageToWriter
//
// Gets all assets in a package of that database and adds them to the package writer
//
void Cooker::AddPackageToWriter(core::HashString const packageId, std::string const& dbPath, PackageWriter &writer, pl::EditorAssetDatabase& db)
{
	// Loop over files - add them to the writer
	pl::EditorAssetDatabase::T_AssetList assets = db.GetAssetsInPackage(packageId);
	std::string const baseAssetPath = dbPath + db.GetAssetPath();
	for (pl::EditorAssetBase* const editorAsset : assets)
	{
		if (!m_GenerateCompiled)
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


} // namespace cooker
} // namespace et
