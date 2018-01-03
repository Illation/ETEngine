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

--setting output directories
INTERMEDIATE = path.join(PROJECT_DIR, "build/Intermediate/")	--intermediate files go in		build/Intermediate/config_platform/project
OUT_DIR = path.join(PROJECT_DIR, "bin/")						--binaries go in				bin/config_platform/project
function outputDirectories(_project)
	local cfgs = configurations()
	local p = platforms()
	for i = 1, #cfgs do
		for j = 1, #p do
			local outDir = cfgs[i] .. "_" .. p[j] .. "/" .. _project
			configuration { cfgs[i], p[j] }
				targetdir = path.join(OUT_DIR, outDir)
				objdir = path.join(INTERMEDIATE, outDir)  
		end
	end
	configuration {}
end

--platform specific library paths
function platformLibraries()
	local p = platforms()
	for j = 1, #p do
		local depPf = path.join(DEP_DIR, p[j] .. "/") 

		configuration { "vs*", p[j] }
			libdirs { path.join(depPf, "sdl2"),path.join(depPf, "freeImage"), path.join(depPf, "assimp") }
	end
	configuration {}
end

--copy files that are specific for the platform being built for
function windowsPlatformPostBuild()
	local p = platforms()
	for j = 1, #p do
		local copyCmd = "$(SolutionDir)..\\build\\copyResources_windows.bat \"$(SolutionDir)\" \"$(OutDir)\" \"" .. p[j] .. "\""

		configuration { "vs*", p[j] }
			--copy dlls and resources after build
			postbuildcommands { copyCmd }
			--copy dlls and resources when running in debugger -> restarting game will copy any changed shaders over
			custombuildtask { {
				"",						--input file
				"randomBullshit.svg",	--output file
				{},						--additional dependencies 
				{copyCmd }				--commands
			} }
	end
	configuration {}
end

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
	includedirs { path.join(DEP_INCLUDE, "sdl2"),path.join(DEP_INCLUDE, "freeImage"), path.join(DEP_INCLUDE, "assimp") }
	debugdir "$(OutDir)"
configuration { "linux", "gmake"}
	defines { "PLATFORM_Linux", "__linux__" }
	includedirs { "/usr/include" }
	buildoptions_cpp { "-std=c++14" }
configuration {}

startproject "DemoGenerated"

project "DemoGenerated"
	kind "ConsoleApp"

	location "../source/Demo"

    defines { "_CONSOLE" }

	--Set output folders
	outputDirectories("Demo")

	--WINDOWS
	configuration "vs*"
		flags { "Winmain"}

		links { "opengl32", "SDL2main" } 

	platformLibraries()
	windowsPlatformPostBuild()

	--Linked libraries
    links{ "EngineGenerated", "SDL2", "FreeImage", "assimp" }

	--additional includedirs
	local ProjBase = path.join(SOURCE_DIR, "Demo") 
	includedirs { path.join(ProjBase, "../Demo"), path.join(ProjBase, "Materials"), path.join(ProjBase, "Scenes") }

	--Source files
    files { 
		path.join(SOURCE_DIR, "Demo/**.cpp"), 
		path.join(SOURCE_DIR, "Demo/**.hpp"), 
		path.join(SOURCE_DIR, "Demo/**.h"), 
		path.join(SOURCE_DIR, "Demo/**.glsl"), 
	}

project "EngineGenerated"
    kind "StaticLib"

	location "../source/Engine"
	
	outputDirectories("Engine")

    files { 
		path.join(SOURCE_DIR, "Engine/**.cpp"), 
		path.join(SOURCE_DIR, "Engine/**.hpp"), 
		path.join(SOURCE_DIR, "Engine/**.h"), 
		path.join(SOURCE_DIR, "Engine/**.glsl"), 
	}

	--additional includedirs
	local ProjBase = path.join(SOURCE_DIR, "Engine") 
	includedirs { path.join(ProjBase, "Base"), path.join(ProjBase, "Components"), path.join(ProjBase, "Content"), path.join(ProjBase, "Helper"), 
		path.join(ProjBase, "Graphics"), path.join(ProjBase, "Prefabs"), path.join(ProjBase, "SceneGraph"), path.join(ProjBase, "Framebuffers"), 
		path.join(ProjBase, "GraphicsHelper"), path.join(ProjBase, "Materials"), path.join(ProjBase, "Math"), path.join(ProjBase, "PlanetTech"), 
		path.join(ProjBase, "PlanetTech/Types"), path.join(ProjBase, "StaticDependancies/glad"), path.join(ProjBase, "../Engine")}

	pchheader "stdafx.hpp"
	pchsource "../source/Engine/stdafx.cpp"

project "TestingGenerated"
	kind "ConsoleApp"

	location "../source/Testing"

    defines { "_CONSOLE" }

	outputDirectories("Testing")

	configuration "vs*"
		flags { "Winmain"}
		debugdir "$(SolutionDir)"
		includedirs { path.join(DEP_INCLUDE, "catch") }
		links { "opengl32", "SDL2main" } 

	platformLibraries()

    files { path.join(SOURCE_DIR, "Testing/**.cpp") }

    links{ "EngineGenerated", "SDL2", "FreeImage", "assimp" }