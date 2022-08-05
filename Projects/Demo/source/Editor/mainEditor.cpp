#include <EtFramework/stdafx.h>

#include <glibmm/refptr.h>
#include <glibmm/miscutils.h>

#include <_generated/resources.c>

#include <EtCore/Platform/AtomicTypes.h>
#include <EtCore/Platform/PlatformUtil.h>
#include <EtCore/FileSystem/FileUtil.h>

#include <EtEditor/EditorApp.h>
#include <EtEditor/Util/EditorConfig.h>

#include <Pipeline/linkerHelper.h>

#include <Common/DemoUI.h>

#ifdef ET_PLATFORM_WIN
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#endif


// forward
void SetDebuggingOptions();


//---------------------------------
// main
//
// Entry point for the program
//
int main(int argc, char *argv[])
{
	using namespace et::demo;

	// Memory leak checks
	SetDebuggingOptions();

	ForceLinkingPipeline();

	// working dir
	if (argc > 0)
	{
		et::core::FileUtil::SetExecutablePath(argv[0]);
	}
	else
	{
		std::cerr << "main > Couldn't get extract working directory from arguments, exiting!" << std::endl;
		return 1;
	}

	// compiled resources
	editor_register_resource();

	// Since this example is running uninstalled, we have to help it find its
	// schema. This is *not* necessary in a properly installed application.
	Glib::setenv("GSETTINGS_SCHEMA_DIR", ".", false);
	Glib::setenv("XDG_DATA_DIRS", et::core::FileUtil::GetAbsolutePath("../share/"), false);

	Glib::RefPtr<et::edit::EditorApp> editorApp = et::edit::EditorApp::create();
	if (!(et::edit::EditorConfig::GetInstance()->GetStartScene().IsEmpty()))
	{
		et::demo::DemoUI::SetupDataModels();
		// Start the application, showing the initial window,
		// and opening extra views for any files that it is asked to open,
		// for instance as a command-line parameter.
		// run() will return when the last window has been closed.
		et::int32 result = editorApp->run(argc, argv);

		// compiled resources
		editor_unregister_resource();

		return result;
	}
	else
	{
		std::cout << "main > Couldn't determine start scene to launch with!" << std::endl;
		return 2;
	}
}

//---------------------------------
// SetDebuggingOptions
//
// Memory checks in debug mode
//
void SetDebuggingOptions()
{
	//notify user if heap is corrupt
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	// Enable run-time memory leak check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	typedef HRESULT(__stdcall *fPtr)(const IID&, void**);

	//_CrtSetBreakAlloc( 87 );
#endif
}

#ifdef ET_PLATFORM_WIN
//---------------------------------
// WinMain
//
// On windows this will be our entry point instead - reroute to the regular main function
//
int APIENTRY WinMain(HINSTANCE, HINSTANCE, PSTR, int)
{
	std::vector<char*> argv = et::core::platform::GetCommandlineArgV();
	return main(static_cast<int>(argv.size()), &argv[0]);
}
#endif
