solution "ETEngineGenerated"
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

    location "../source/"
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


project "DemoGenerated"
	kind "ConsoleApp"

	location "../source/Demo"

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

    files { 
		path.join(SOURCE_DIR, "Demo/**.cpp"), 
		path.join(SOURCE_DIR, "Demo/**.hpp"), 
		path.join(SOURCE_DIR, "Demo/**.h"), 
	}

    links{
		"Engine",
    	"SDL2",
		"ILU"
    }

project "EngineGenerated"
    kind "StaticLib"

	location "../source/Engine"

    files { 
		path.join(SOURCE_DIR, "Engine/**.cpp"), 
		path.join(SOURCE_DIR, "Engine/**.hpp"), 
		path.join(SOURCE_DIR, "Engine/**.h"), 
	}

    --excludes { path.join(SOURCE_DIR, "Screenshot.cpp") }

    links{
    	"SDL2",
		"ILU"
    }
    --pchheader path.join(SOURCE_DIR, "stdafx.h")
    --pchsource path.join(SOURCE_DIR, "stdafx.cpp")
