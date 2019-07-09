# Building this project


### Building Bullet Physics
The Engine uses Bullet Physics and has prebuilt libfiles for Visual Studio 2015. If you want to use a different compiler, you need to build bullet for it, otherwise you can skip this section.

Clone bullet from https://github.com/bulletphysics/bullet3/ and download [CMake](https://cmake.org/).
You will need .lib files for Debug_x32, Release_x32, Debug_x64 and Release_x64, and they need to go in the respective folders in dependancies.

When you generate the project files with cmake, make sure to check the option "USE_MSVC_RUNTIME_LIBRARY_DLL".
Build ALL_BUILD and check the /lib/ directory where you cloned bullet. The files you should copy are:
 * BulletCollsion.lib
 * BulletDynamics.lib
 * LinearMath.lib

Make sure to remove any "_Debug" suffixes from the filenames.

### The Engine
EtEngine uses CMake to generate project files. 
Currently the project is being developed with Visual Studio 2017, but it is done in a way that should make it easy to transfer to other platforms by using project generation and crossplatform libraries

The project comes with packaged lib, dll and include files for all dependancies, compiled with vs2017. If you want to build with a different visual studio version, you need to build GTKmm and its dependancies for that compiler.

In order to build the project, you need to generate the project files with CMake. You should have CMake 3.10 or later installed on your computer.

Start by opening a terminal and navigating to the repository root folder, then execute:

    cmake -G "Visual Studio 15 2017 Win64" -H. -Bbuild

The -H and -B options specify an out of source build in the build folder, which is recommended as the gitignore is set up for this case. You could alternatively do this

    cd build
	cmake -G "Your preferred generator [and architecture]" ..

Next open the solution that was generated at **build/EtEngine.sln** in visual studio, select the desired configuration (_Development_ is good for most purposes), and build as usual.

The final executable will appear in **bin/[configuration]_[platform]/bin/EtEngine.exe**

In order to run the built program, you also need to assemble its dependancies - such as dlls and resource files - relative to the executable in the inner 'bin/' folder. To do this, build the 'INSTALL' project in visual studio.

### Unit Tests

ETEngine uses [catch](https://github.com/catchorg/Catch2/tree/Catch1.x) (currently version 1) to perform unit tests.

To run unit tests, run the _Testing_ target.

For a brief test you can simply run it from Visual Studio and check that the exit code is **0x0**
If you want more details on tests that fail, run the generated executable from a terminal:

    .\bin\[configuaration]_[platform]\Testing\Testing.exe
