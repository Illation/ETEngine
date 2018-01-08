# Extra Terrestial Engine

Space sim focused engine created from a merge of:
 * my [OpenGL Framework](https://github.com/Illation/GLFramework)
 * my [Planet Renderer](https://github.com/Illation/PlanetRenderer)
 
Full feature list and backlog here: [LINK](features.md)

## Build Status

| Service | System | Compiler | Branch | Status |
| ------- | ------ | -------- | ------ | ------ |
| [AppVeyor](https://ci.appveyor.com/project/Illation/etengine)| Windows 32 | Visual Studio 2015 | master | [![AppVeyor](https://ci.appveyor.com/api/projects/status/jsr44exh2l0y5gs6/branch/master?svg=true)](https://ci.appveyor.com/project/Illation/etengine)

## How to build

ETEngine uses a project generation tool called [GENie](https://github.com/bkaradzic/GENie) to generate project files. Currently only project generation for Visual Studio 2015 has been tested, but support for Linux makefiles should soon be added.

In order to build the project, first download the GENie executable. The easiest thing for later use would be installing it in your PATH, but you can also use the executable anywhere else.
Next, open a terminal and navigate to the repository root folder, and execute:

    genie --file=build/genie.lua vs2015

If you didn't install GENie in your path it would look more like

    .\relative\path\to\your\genie.exe --file=build/genie.lua [preferred visual studio version]

Next open the solution that was generated at **build/ETEngine.sln** in visual studio, select the desired configuration (_Development_ is good for most purposes), and build as usual.

The final executable will appear in **bin/[configuration]_[platform]/Demo/Demo.exe**

## Unit Tests

ETEngine uses [catch](https://github.com/catchorg/Catch2/tree/Catch1.x) (currently version 1) to perform unit tests.

To run unit tests, generate the project files as before, and set _Testing_ as your startup project, and compile it. 

For a brief test you can simply run it from Visual Studio and check that the exit code is **0x0**
If you want more details on tests that fail, run the generated executable from a terminal:

    .\bin\[configuaration]_[platform]\Testing\Testing.exe
 
## Acknowledgments

Currently used libraries: SDL2, catch, FreeImage, GLAD and Assimp
Also a bunch of (modified) code from Eric Brunetons [atmospheric scattering implementation](https://github.com/ebruneton/precomputed_atmospheric_scattering).

for library licenses see Library Licenses.md
