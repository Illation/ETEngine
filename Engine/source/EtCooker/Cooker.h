#pragma once
#include <EtPipeline/Core/Content/EditorAssetDatabase.h>

#include "PackageWriter.h"
#include "CookerFwd.h"


namespace et { namespace pl {
	class FileResourceManager;
}  namespace app {
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
		FailedToSerialize,
		FailedToCleanup,
		FailedToWritePackage,
		FailedToAccessGeneratedFile
	};

	typedef std::function<void(core::PackageDescriptor const* const desc, PackageWriter& writer)> T_PreWritePackageFn; // desc is nullptr for default pkg

	// construct destruct
	//--------------------
	Cooker(int32 const argc, char* const argv[]);
	~Cooker();

	// functionality
	//---------------
	void Run();

	void RegisterPreWritePackageFn(T_PreWritePackageFn const& fn);

	// accessors
	//-----------
	E_ReturnCode GetReturnCode() const { return m_ReturnCode; }

	pl::FileResourceManager const* GetResourceManager() const { return m_ResMan.Get(); }

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

	core::Directory* m_TempDir = nullptr;

	UniquePtr<app::GlfwRenderWindow> m_RenderWindow;
	Ptr<pl::FileResourceManager> m_ResMan;

	std::vector<T_PreWritePackageFn> m_PreWritePackageFns;
	
	E_ReturnCode m_ReturnCode = E_ReturnCode::Success;
};


} // namespace cooker
} // namespace et
