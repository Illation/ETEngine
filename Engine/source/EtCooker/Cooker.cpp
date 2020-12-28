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

#include <EtRuntime/Rendering/GlfwRenderWindow.h>

#include <EtPipeline/Content/FileResourceManager.h>


namespace et {
namespace cooker {


//============
// E T Cooker
//============


// static
std::string const Cooker::s_TempPath = "temp";


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

	std::string projectPath(argv[1]);
	std::string enginePath(argv[2]);
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

	// Graphics context
	m_RenderWindow = new rt::GlfwRenderWindow(true);
	render::ContextHolder::Instance().CreateMainRenderContext(m_RenderWindow); 

	// resources
	m_ResMan = new pl::FileResourceManager(projectPath, enginePath);
	core::ResourceManager::SetInstance(m_ResMan);

	// Ensure the generated file directory exists
	m_TempDir = new core::Directory(s_TempPath, nullptr, true);
}

//---------------
// Cooker::d-tor
//
Cooker::~Cooker()
{
	if (!m_TempDir->Delete())
	{
		LOG("CookCompiledPackage > Failed to clean up temporary file directory!", core::LogLevel::Error);
		delete m_TempDir;
		m_ReturnCode = E_ReturnCode::FailedToCleanup;
	}

	m_TempDir = nullptr;

	core::ResourceManager::DestroyInstance();

	m_RenderWindow->GetArea().Uninitialize();
	delete m_RenderWindow;
	m_RenderWindow = nullptr;

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
		m_ResMan->GetProjectDatabase().SetupAllRuntimeAssets();
		m_ResMan->GetEngineDatabase().SetupAllRuntimeAssets();

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
	std::string const dbName(core::ResourceManager::s_DatabasePath);
	std::string const tempDbFullPath = s_TempPath + std::string("/") + dbName;

	core::AssetDatabase mergeDb(false);
	m_ResMan->GetProjectDatabase().PopulateAssetDatabase(mergeDb);
	m_ResMan->GetEngineDatabase().PopulateAssetDatabase(mergeDb);
	if (!core::serialization::SerializeToFile(tempDbFullPath, mergeDb))
	{
		LOG(FS("CookCompiledPackage > Failed to serialize asset database to '%s'", tempDbFullPath.c_str()), core::LogLevel::Error);
		m_ReturnCode = E_ReturnCode::FailedToSerialize;
		return;
	}

	// load the asset database from the temporary file and add it to the package
	core::File* dbFile = new core::File(dbName, m_TempDir);
	packageWriter.AddFile(dbFile, dbFile->GetPath(), core::E_CompressionType::Store);

	// add the boot config
	core::File* cfgFile = new core::File(m_ResMan->GetProjectPath() + fw::BootConfig::s_FileName, nullptr);
	packageWriter.AddFile(cfgFile, m_ResMan->GetProjectPath(), core::E_CompressionType::Store);

	// add all other compiled files to the package
	static core::HashString const s_CompiledPackageId;
	AddPackageToWriter(s_CompiledPackageId, m_ResMan->GetProjectPath(), packageWriter, m_ResMan->GetProjectDatabase());
	AddPackageToWriter(s_CompiledPackageId, m_ResMan->GetEnginePath(), packageWriter, m_ResMan->GetEngineDatabase());

	// write our package
	packageWriter.Write(packageData);

	// Generate source file
	GenerateCompilableResource(packageData, m_ResourceName, m_OutPath);

	// cleanup
	packageWriter.RemoveFile(dbFile);
}

//-----------------------------
// Cooker::CookCompiledPackage
//
// Writes all packages that don't get generated backed into the executable 
//
void Cooker::CookFilePackages()
{
	// Get a unified list of package descriptors
	std::vector<core::PackageDescriptor> descriptors = m_ResMan->GetProjectDatabase().GetPackages();
	for (core::PackageDescriptor const& desc : m_ResMan->GetEngineDatabase().GetPackages())
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

		AddPackageToWriter(desc.GetId(), m_ResMan->GetProjectPath(), packageWriter, m_ResMan->GetProjectDatabase());
		AddPackageToWriter(desc.GetId(), m_ResMan->GetEnginePath(), packageWriter, m_ResMan->GetEngineDatabase());

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

		editorAsset->Generate(m_Configuration, m_TempDir);

		std::vector<pl::EditorAssetBase::RuntimeAssetInfo> const runtimeAssets = editorAsset->GetAllRuntimeAssets();
		for (pl::EditorAssetBase::RuntimeAssetInfo const& info : runtimeAssets)
		{
			core::I_Asset const* const asset = info.m_Asset;
			if (info.m_HasGeneratedData)
			{
				core::Entry* const genEntry = m_TempDir->GetMountedChild(asset->GetPath() + asset->GetName());
				if ((genEntry == nullptr) || (genEntry->GetType() == core::Entry::ENTRY_DIRECTORY))
				{
					LOG(FS("CookCompiledPackage > Failed to access generated asset file '%s'", 
						(asset->GetPath() + asset->GetName()).c_str()), 
						core::LogLevel::Warning);
					m_ReturnCode = E_ReturnCode::FailedToAccessGeneratedFile;
					continue;
				}

				core::File* const assetFile = static_cast<core::File*>(genEntry);
				writer.AddFile(assetFile, m_TempDir->GetName(), core::E_CompressionType::Store, false);
			}
			else
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
}


} // namespace cooker
} // namespace et
