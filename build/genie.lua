solution "ETEngineGenerated"
    configurations {
        "Debug",
        "DebugEditor",
        "Development",
        "DevelopmentEditor",
		"Shipping"
    }

    platforms {
        "x64",
        "x32"
    }

    language "C++"

    location "../source/"
    objdir "../build/"
	

PROJECT_DIR = "../"
SOURCE_DIR = path.join(PROJECT_DIR, "source/")
DEP_DIR = path.join(PROJECT_DIR, "dependancies/")
DEP_INCLUDE = path.join(DEP_DIR, "include/")
DEP_X32 = path.join(DEP_DIR, "x32/")
DEP_X64 = path.join(DEP_DIR, "x64/")
INTERMEDIATE = "Intermediate/"
OUT_DIR = path.join(PROJECT_DIR, "bin/") 

files { 
	path.join(PROJECT_DIR, "build/copyResources_windows.bat"), 
	path.join(PROJECT_DIR, "build/genie.lua"), 
}

configuration "Debug"
	defines { "_DEBUG" }
	flags { "Symbols", "ExtraWarnings" }
configuration "DebugEditor"
	defines { "_DEBUG", "EDITOR" }
	flags { "Symbols", "ExtraWarnings" }
configuration "Development"
	flags {"OptimizeSpeed", "Symbols" }
configuration "DevelopmentEditor"
	defines { "EDITOR" }
	flags {"OptimizeSpeed", "Symbols" }
configuration "Shipping"
	flags {"OptimizeSpeed", "No64BitChecks" }

configuration "vs*"
	flags { "Unicode" }
	defines { "WIN32", "PLATFORM_Win" }
	debugdir "$(OutDir)"
	links { "DevIL", "ILUT" }
configuration { "linux", "gmake"}
	defines { "PLATFORM_Linux", "__linux__" }
	includedirs { "/usr/include" }
	buildoptions_cpp { "-std=c++14" }
	links { "IL" }
configuration {}


project "DemoGenerated"
	kind "ConsoleApp"

	location "../source/Demo"

    defines { "_CONSOLE" }

	configuration { "Debug", "x32" }
		targetdir path.join(OUT_DIR, "Debug_x32/Demo")
		objdir path.join(INTERMEDIATE, "Debug_x32/Demo")  
	configuration { "Debug", "x64" }
		targetdir path.join(OUT_DIR, "Debug_x64/Demo")
		objdir path.join(INTERMEDIATE, "Debug_x64/Demo")
	configuration { "DebugEditor", "x32" }
		targetdir path.join(OUT_DIR, "DebugEditor_x32/Demo")
		objdir path.join(INTERMEDIATE, "DebugEditor_x32/Demo")  
	configuration { "DebugEditor", "x64" }
		targetdir path.join(OUT_DIR, "DebugEditor_x64/Demo")
		objdir path.join(INTERMEDIATE, "DebugEditor_x64/Demo")    
	configuration { "Development", "x32" }
		targetdir path.join(OUT_DIR, "Development_x32/Demo")
		objdir path.join(INTERMEDIATE, "Development_x32/Demo")  
	configuration { "Development", "x64" }
		targetdir path.join(OUT_DIR, "Development_x64/Demo")
		objdir path.join(INTERMEDIATE, "Development_x64/Demo") 
	configuration { "DevelopmentEditor", "x32" }
		targetdir path.join(OUT_DIR, "DevelopmentEditor_x32/Demo")
		objdir path.join(INTERMEDIATE, "DevelopmentEditor_x32/Demo")  
	configuration { "DevelopmentEditor", "x64" }
		targetdir path.join(OUT_DIR, "DevelopmentEditor_x64/Demo")
		objdir path.join(INTERMEDIATE, "DevelopmentEditor_x64/Demo")       
	configuration { "Shipping", "x32" }
		targetdir path.join(OUT_DIR, "Shipping_x32/Demo")
		objdir path.join(INTERMEDIATE, "Shipping_x32/Demo")  
	configuration { "Shipping", "x64" }
		targetdir path.join(OUT_DIR, "Shipping_x64/Demo")
		objdir path.join(INTERMEDIATE, "Shipping_x64/Demo")   

	configuration "vs*"
		flags { "Winmain"}

		includedirs { path.join(DEP_INCLUDE, "sdl2"),path.join(DEP_INCLUDE, "freeImage"), path.join(DEP_INCLUDE, "assimp") }
		links { "opengl32", "SDL2main" } 
		postbuildcommands { "$(SolutionDir)..\\build\\copyResources_windows.bat \"$(SolutionDir)\" \"$(OutDir)\"" }

		custombuildtask { {
			"",																							--input file
			"randomBullshit.svg",																		--output file
			{},																							--additional dependencies 
			{"$(SolutionDir)..\\build\\copyResources_windows.bat \"$(SolutionDir)\" \"$(OutDir)\"" }	--commands
		} }
	configuration { "vs*", "x32" }
		libdirs { path.join(DEP_X32, "sdl2"),path.join(DEP_X32, "freeImage"), path.join(DEP_X32, "assimp") }
	configuration { "vs*", "x64" }
		libdirs { path.join(DEP_X64, "sdl2"),path.join(DEP_X64, "freeImage"), path.join(DEP_X64, "assimp") }
    configuration {}

    files { 
		path.join(SOURCE_DIR, "Demo/**.cpp"), 
		path.join(SOURCE_DIR, "Demo/**.hpp"), 
		path.join(SOURCE_DIR, "Demo/**.h"), 
		path.join(SOURCE_DIR, "Demo/**.glsl"), 
	}

    links{ "Engine", "SDL2", "FreeImage", "assimp" }

project "EngineGenerated"
    kind "StaticLib"

	location "../source/Engine"
	
	configuration { "Debug", "x32" }
		targetdir path.join(OUT_DIR, "Debug_x32/Engine")
		objdir path.join(INTERMEDIATE, "Debug_x32/Engine")  
	configuration { "Debug", "x64" }
		targetdir path.join(OUT_DIR, "Debug_x64/Engine")
		objdir path.join(INTERMEDIATE, "Debug_x64/Engine")  
	configuration { "DebugEditor", "x32" }
		targetdir path.join(OUT_DIR, "DebugEditor_x32/Engine")
		objdir path.join(INTERMEDIATE, "DebugEditor_x32/Engine")  
	configuration { "DebugEditor", "x64" }
		targetdir path.join(OUT_DIR, "DebugEditor_x64/Engine")
		objdir path.join(INTERMEDIATE, "DebugEditor_x64/Engine")    
	configuration { "Development", "x32" }
		targetdir path.join(OUT_DIR, "Development_x32/Engine")
		objdir path.join(INTERMEDIATE, "Development_x32/Engine")  
	configuration { "Development", "x64" }
		targetdir path.join(OUT_DIR, "Development_x64/Engine")
		objdir path.join(INTERMEDIATE, "Development_x64/Engine") 
	configuration { "DevelopmentEditor", "x32" }
		targetdir path.join(OUT_DIR, "DevelopmentEditor_x32/Engine")
		objdir path.join(INTERMEDIATE, "DevelopmentEditor_x32/Engine")  
	configuration { "DevelopmentEditor", "x64" }
		targetdir path.join(OUT_DIR, "DevelopmentEditor_x64/Engine")
		objdir path.join(INTERMEDIATE, "DevelopmentEditor_x64/Engine")   
	configuration { "Shipping", "x32" }
		targetdir path.join(OUT_DIR, "Shipping_x32/Engine")
		objdir path.join(INTERMEDIATE, "Shipping_x32/Engine")  
	configuration { "Shipping", "x64" }
		targetdir path.join(OUT_DIR, "Shipping_x64/Engine")
		objdir path.join(INTERMEDIATE, "Shipping_x64/Engine")   

	configuration "vs*"
		includedirs { path.join(DEP_INCLUDE, "sdl2"),path.join(DEP_INCLUDE, "freeImage"), path.join(DEP_INCLUDE, "assimp") }
    configuration {}

    files { 
		path.join(SOURCE_DIR, "Engine/**.cpp"), 
		path.join(SOURCE_DIR, "Engine/**.hpp"), 
		path.join(SOURCE_DIR, "Engine/**.h"), 
		path.join(SOURCE_DIR, "Engine/**.glsl"), 
	}

	pchheader "stdafx.hpp"
	pchsource "stdafx.cpp"


project "TestingGenerated"
	kind "ConsoleApp"

	location "../source/Testing"

    defines { "_CONSOLE" }

	configuration { "Debug", "x32" }
		targetdir path.join(OUT_DIR, "Debug_x32/Testing")
		objdir path.join(INTERMEDIATE, "Debug_x32/Testing")  
	configuration { "Debug", "x64" }
		targetdir path.join(OUT_DIR, "Shipping_x64/Testing")
		objdir path.join(INTERMEDIATE, "Shipping_x64/Testing")  
	configuration { "Development", "x32" }
		targetdir path.join(OUT_DIR, "Development_x32/Testing")
		objdir path.join(INTERMEDIATE, "Development_x32/Testing")  
	configuration { "Development", "x64" }
		targetdir path.join(OUT_DIR, "Development_x64/Testing")
		objdir path.join(INTERMEDIATE, "Development_x64/Testing")    

	configuration "vs*"
		flags { "Winmain"}
		debugdir "$(SolutionDir)"
		includedirs { path.join(DEP_INCLUDE, "sdl2"),path.join(DEP_INCLUDE, "freeImage"), path.join(DEP_INCLUDE, "assimp"), path.join(DEP_INCLUDE, "catch") }
		links { "opengl32", "SDL2main" } 
	configuration { "vs*", "x32" }
		libdirs { path.join(DEP_X32, "sdl2"),path.join(DEP_X32, "freeImage"), path.join(DEP_X32, "assimp") }
	configuration { "vs*", "x64" }
		libdirs { path.join(DEP_X64, "sdl2"),path.join(DEP_X64, "freeImage"), path.join(DEP_X64, "assimp") }
    configuration {}

    files { path.join(SOURCE_DIR, "Testing/**.cpp") }

    links{ "Engine", "SDL2", "FreeImage", "assimp" }