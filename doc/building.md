# Building this project


### TLDR

    cd Projects/Demo
    cmake -G "Visual Studio 15 2017 Win64" -S . -B build
    cmake --build build --target all --config Develop
	cmake --build build --target install
	cmake --build build --target cook-installed-resources-EtEngineDemo


### Generating the project files

EtEngine uses **CMake** to generate project files. You should at least have CMake 3.10 installed.

Currently the project is being developed with Visual Studio 2017, but it is done in a way that should make it easy to transfer to other platforms by using project generation and crossplatform libraries

In order to build the project, you need to generate the project files with CMake. You should have CMake 3.10 or later installed on your computer.

Start by opening a terminal and navigating to the project root folder (**Projects/Demo/**), then execute:

    cmake -G "Visual Studio 15 2017 Win64" -S . -B build

The -H and -B options specify an out of source build in the **build/** folder, which is recommended required. You could alternatively do this

    cd build
	cmake -G "Your preferred generator [and architecture]" ..

##### Known issues:

 * For building gtkmm, vcpkg requires you have the english language pack installed:
https://github.com/microsoft/vcpkg/issues/3842
 * Don't clone recursive, submodule download is handled automatically during project generation


### Third party libraries

The first time the CMake script runs, it will download all submodules, and build the necessary libraries in **Engine/third_party/** with the compiler selected in the main CMake command.
This can take a while, but it only happens once, and afterwards the libraries build files will be reused when you run the CMake script again.

Most of them output the library files in a folder called "**build/**" adjacent to their respective submodule, for example "**Engine/third_party/rttr/build/**". 

If you need to rebuild one of these third party libraries, simply delete the build folder and run the CMake script again.

By default, each library is build for both Debug and Release configurations. If you want to accelerate the process by only building for one of these configurations,
you can turn the CMake option **ETE_SINGLE_CONFIG** on, and specify **ETE_BUILD_LIB_CONFIG** to your desired configuration. For example:

    cmake -G "Visual Studio 15 2017 Win64" -DETE_SINGLE_CONFIG=ON -DETE_BUILD_LIB_CONFIG=Release -S . -B build


### Creating a new Project

You can keep your projects outside of the engine repositiory.

If you wish to create an external project instead of compiling the Demo project, copy the demo projects folder to your desired location. 
To generate the project files, you will need to provide the cmake script with the project location. This can be done with the **ENGINE_DIRECTORY** argument, like so:

    cmake -G "Visual Studio 15 2017 Win64" -DENGINE_DIRECTORY="path/to/engine/repo/Engine" -S . -B build


### Building the project

Next open the solution that was generated at **project_dir/build/** in visual studio, select the desired configuration (_Development_ is good for most purposes), and build as usual.
Alternatively, run this command from the project root directory:

    cmake --build build --target all --config Develop

The final executable will appear in **project_dir/bin/[configuration]_[platform]/ProjectName/ProjectName.exe**


In order to run the built program, you also need to assemble its dependancies - such as dlls and config files - relative to the executable. To do this, build the 'INSTALL' project in visual studio.
Alternatively, run this command from the project root directory:

    cmake --build build --target install

Finally, you will also need to build your programs content:


### Cooking content

The engine loads assets from package files (.etpak). Therefore, all assets need to be cooked into those packages to run the application.
Packages are built by a program called **EtCooker.exe** which is automatically built and installed next to your project folder while building the rest of the project.

Packages come in two flavours: external _.etpak_ files, and resources compiled directly into your project. The latter step is done automatically while building the project, but external files need to be built manually.

To do so, a target will be defined for your project called "**cook-installed-resources-[yourProjectName]**".
You can build this target in visual studio, or by running a cmake build command:

    cmake --build build --target cook-installed-resources-EtEngineDemo

This build step will merge the engine asset database and your projects asset database into a series of _.etpak_ files and install them in your projects binary directory.
The resources that are built are specified in the following files:

 * repo_root/Projects/Demo/resources/asset_database.json
 * repo_root/Engine/resources/asset_database.json


### Unit Tests

ETEngine uses **Catch2** to perform unit tests.

To run unit tests, run the _unit_tests_ target.
You need to specify the unit test source directory in the command as an argument.

For a brief test you can simply run it from Visual Studio and check that the exit code is **0x0**

If you want more details on tests that fail, run the generated executable from a terminal:

    .\bin\[configuaration]_[platform]\unit_tests\unit_tests.exe path/to/engine/repo/Engine/unit_tests/_
