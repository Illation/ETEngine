#include <EtApplication/stdafx.h>
#include "Cooker.h"

#include "CompiledDataGenerator.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/FileSystem/FileUtil.h>
#include <EtCore/FileSystem/Entry.h>
#include <EtCore/FileSystem/Package/FilePackage.h>
#include <EtCore/Reflection/Serialization.h>
#include <EtCore/Reflection/TypeInfoRegistry.h>
#include <EtCore/Content/AssetDatabase.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtCore/Trace/ConsoleTraceHandler.h>
#include <EtCore/Trace/DebugOutputTraceHandler.h>
#include <EtCore/Trace/FileTraceHandler.h>
#include <EtCore/Trace/NetworkTraceHandler.h>

#include <EtRHI/GraphicsContext/ContextHolder.h>

#include <EtApplication/Rendering/GlfwRenderWindow.h>
#include <EtApplication/Core/PackageResourceManager.h>

#include <EtPipeline/Core/Content/FileResourceManager.h>


namespace et {
namespace cooker {


//============
// E T Cooker
//============


// static
std::string const Cooker::s_TempPath = "temp/";


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

	std::string projectPath(core::FileUtil::GetAbsolutePath(argv[1]));
	std::string enginePath(core::FileUtil::GetAbsolutePath(argv[2]));
	m_OutPath = core::FileUtil::GetAbsolutePath(argv[3]);
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
	core::TraceService::Initialize(); 

	core::TraceService::Instance()->AddHandler<core::FileTraceHandler>("cooker.log"); // File trace first in case network trace fails
	core::TraceService::Instance()->SetupDefaultHandlers(FS("ET Cooker (%s)", m_GenerateCompiled ? "Compiled" : "Package"), false);

	core::TypeInfoRegistry::Instance().Initialize(); 

	ET_ASSERT(m_GenerateCompiled || (std::string(argv[4]) == "n"), "Expected argument 4 to be either 'y' or 'n'!");

	ET_LOG_I(ET_CTX_COOKER, "E.T.Cooker");
	ET_LOG_I(ET_CTX_COOKER, "//////////");
	ET_LOG_I(ET_CTX_COOKER, "");
	ET_LOG_I(ET_CTX_COOKER, " - version: %s", build::Version::s_Name.c_str());
	ET_LOG_I(ET_CTX_COOKER, "");

	// Graphics context
	m_RenderWindow = Create<app::GlfwRenderWindow>(core::WindowSettings(), true);
	rhi::ContextHolder::Instance().CreateMainRenderContext(m_RenderWindow);

	// resources
	UniquePtr<pl::FileResourceManager> resMan = Create<pl::FileResourceManager>(projectPath, enginePath);
	m_ResMan = ToPtr(resMan.Get());
	core::ResourceManager::SetInstance(std::move(resMan));

	// Ensure the generated file directory exists
	m_TempDir = new core::Directory(s_TempPath, nullptr, true);
	m_TempDir->Mount(true);
}

//---------------
// Cooker::d-tor
//
Cooker::~Cooker()
{
	if (!m_TempDir->Delete())
	{
		ET_LOG_E(ET_CTX_COOKER, "CookCompiledPackage > Failed to clean up temporary file directory!");
		delete m_TempDir;
		m_ReturnCode = E_ReturnCode::FailedToCleanup;
	}

	m_TempDir = nullptr;

	core::ResourceManager::DestroyInstance();

	m_RenderWindow->GetArea().Uninitialize();
	m_RenderWindow = nullptr;

	core::TraceService::Destroy();
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

//-----------------------------------
// Cooker::RegisterPreWritePackageFn
//
void Cooker::RegisterPreWritePackageFn(T_PreWritePackageFn const& fn)
{
	m_PreWritePackageFns.emplace_back(fn);
}

//-----------------------------
// Cooker::CookCompiledPackage
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
	std::string const dbName(app::PackageResourceManager::s_DatabasePath);
	std::string const tempDbFullPath = s_TempPath + dbName;

	core::AssetDatabase mergeDb(false);
	m_ResMan->GetProjectDatabase().PopulateAssetDatabase(mergeDb);
	m_ResMan->GetEngineDatabase().PopulateAssetDatabase(mergeDb);

	bool const isVerbose = (m_Configuration.m_Configuration != pl::BuildConfiguration::E_Configuration::Shipping);
	if (!core::serialization::SerializeToFile(tempDbFullPath, mergeDb, isVerbose))
	{
		ET_LOG_E(ET_CTX_COOKER, "CookCompiledPackage > Failed to serialize asset database to '%s'", tempDbFullPath.c_str());
		m_ReturnCode = E_ReturnCode::FailedToSerialize;
		return;
	}

	// load the asset database from the temporary file and add it to the package
	core::File* dbFile = new core::File(dbName, m_TempDir);
	packageWriter.AddFile(dbFile, dbFile->GetPath(), core::E_CompressionType::Store);

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
			ET_LOG_E(ET_CTX_COOKER, "CookFilePackages > Failed to open file '%s'", outFile->GetName());
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

		editorAsset->Generate(m_Configuration, m_TempDir, baseAssetPath);

		std::vector<pl::EditorAssetBase::RuntimeAssetInfo> const runtimeAssets = editorAsset->GetAllRuntimeAssets();
		for (pl::EditorAssetBase::RuntimeAssetInfo const& info : runtimeAssets)
		{
			core::I_Asset const* const asset = info.m_Asset.Get();
			if (info.m_HasGeneratedData)
			{
				core::Entry* const genEntry = m_TempDir->GetMountedChild(asset->GetPath() + asset->GetName());
				if ((genEntry == nullptr) || (genEntry->GetType() == core::Entry::ENTRY_DIRECTORY))
				{
					ET_LOG_E(ET_CTX_COOKER, 
						"CookCompiledPackage > Failed to access generated asset file '%s'", 
						(asset->GetPath() + asset->GetName()).c_str());
					m_ReturnCode = E_ReturnCode::FailedToAccessGeneratedFile;
					continue;
				}

				ET_LOG_I(ET_CTX_COOKER, "%s [%u] @: %s", asset->GetId().ToStringDbg(), asset->GetId().Get(), genEntry->GetName().c_str());

				core::File* const assetFile = static_cast<core::File*>(genEntry);
				writer.AddFile(assetFile, m_TempDir->GetName(), core::E_CompressionType::Store, false);
			}
			else
			{
				std::string const filePath = baseAssetPath + asset->GetPath();
				std::string const assetName = asset->GetName();
				core::HashString const id = asset->GetId();

				ET_LOG_I(ET_CTX_COOKER, "%s [%u] @: %s", id.ToStringDbg(), id.Get(), core::FileUtil::GetAbsolutePath(filePath).c_str());

				core::File* const assetFile = new core::File(filePath + assetName, nullptr);
				writer.AddFile(assetFile, baseAssetPath, core::E_CompressionType::Store);
			}
		}
	}
}


} // namespace cooker
} // namespace et
