#pragma once
#include <EtPipeline/Content/EditorAssetDatabase.h>

#include "PackageWriter.h"


namespace et { namespace pl {
	class FileResourceManager;
}  namespace rt {
	class GlfwRenderWindow;
} }


namespace et { 
namespace cooker {


//---------
// Cooker
//
// Main class for the cooker program. Essentially all the work can be done by calling Run
//
class Cooker final
{
	// definitions
	//-------------
	static std::string const s_TempPath;

public:
	enum class E_ReturnCode
	{
		Success = 0,

		InsufficientArguments,
		MissingResourceName,
		FailedToSerialize,
		FailedToCleanup,
		FailedToWritePackage,
		FailedToAccessGeneratedFile
	};


	// construct destruct
	//--------------------
	Cooker(int32 const argc, char* const argv[]);
	~Cooker();

	// functionality
	//---------------
	void Run();

	// accessors
	//-----------
	E_ReturnCode GetReturnCode() const { return m_ReturnCode; }

	// utility
	//---------
private:
	void CookCompiledPackage();
	void CookFilePackages();

	void AddPackageToWriter(core::HashString const packageId, std::string const& dbPath, PackageWriter &writer, pl::EditorAssetDatabase& db);

	// Data
	///////

	pl::BuildConfiguration m_Configuration;

	bool m_GenerateCompiled;
	std::string m_ResourceName;

	core::Directory* m_TempDir = nullptr;

	std::string m_OutPath;

	rt::GlfwRenderWindow* m_RenderWindow = nullptr;
	pl::FileResourceManager* m_ResMan = nullptr;
	
	E_ReturnCode m_ReturnCode = E_ReturnCode::Success;
};


} // namespace cooker
} // namespace et
