solution "ETEngine"
    configurations {
        "Release",
        "Debug",
		"Shipping"
    }

    platforms {
        "x64",
        "x32",
        "Native" -- for targets where bitness is not specified
    }

    language "C++"

    location "../"
    objdir "../build/"

PROJECT_DIR = "../"
SOURCE_DIR = path.join(PROJECT_DIR, "src/")

configuration "Debug"
	targetdir "../bin/debug/"
	objdir "obj/debug"
	defines { "_DEBUG" }
	flags { "Symbols" }
configuration "Release"
	targetdir "../bin/release/"
	objdir "obj/release"
	flags {"OptimizeSpeed", "Symbols" }
configuration "Shipping"
	targetdir "../bin/shipping/"
	objdir "obj/shipping"
	flags {"OptimizeSpeed", "No64BitChecks" }

configuration "vs*"
	flags { "Unicode" }
	defines { "WIN32", "PLATFORM_Win" }
	debugdir "$(OutDir)"
	links { "DevIL", "ILUT" }
configuration { "linux", "gmake"}
--	flags {"-pedantic"}--
	defines { "PLATFORM_Linux", "__linux__" }
	includedirs { "/usr/include" }
	buildoptions_cpp { "-std=c++14" }
	links { "IL" }
configuration {}


project "Demo"
	kind "ConsoleApp"

    defines { "_CONSOLE" }

	configuration "vs*"
		flags { "Winmain"}
		includedirs { "$(S_LIBS)/SDL2/include", "$(S_LIBS)/DevIL/include", "$(S_LIBS)/glm" }
		libdirs { "$(S_LIBS)/SDL2/lib/x86", "$(S_LIBS)/DevIL/lib/x86/unicode/Release" }
		links { "DevIL", "ILUT" }
		postbuildcommands { 
			"xcopy \"$(SolutionDir)..\\dependancies\\x32\\assimp\\Assimp32.dll\" \"$(OutDir)\" /y/D",
			"xcopy \"$(SolutionDir)..\\dependancies\\x32\\freeImage\\FreeImage.dll\" \"$(OutDir)\" /y/D",
			"xcopy \"$(SolutionDir)..\\dependancies\\x32\\sdl2\\SDL2.dll\" \"$(OutDir)\" /y/D",
			
			"xcopy \"$(SolutionDir)Demo\\Resources\" \"$(OutDir)\\Resources\" /s/i/y",
			"xcopy \"$(SolutionDir)Demo\\Shaders\" \"$(OutDir)\\Shaders\" /s/i/y",

			"xcopy \"$(SolutionDir)Engine\\Shaders\" \"$(OutDir)\\Shaders\" /s/i/y",
		}
    configuration {}

    files { path.join(SOURCE_DIR, "Demo/**.cpp"), }

    links{
		"Engine",
    	"SDL2",
		"ILU"
    }

project "Engine"
    kind "StaticLib"

    files { path.join(SOURCE_DIR, "Engine/**.cpp"), }

    --excludes { path.join(SOURCE_DIR, "Screenshot.cpp") }

    links{
    	"SDL2",
		"ILU"
    }
    --pchheader path.join(SOURCE_DIR, "stdafx.h")
    --pchsource path.join(SOURCE_DIR, "stdafx.cpp")
