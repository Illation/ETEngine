

##############
# Demo
##############


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.glsl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set(_genFile "${CMAKE_CURRENT_SOURCE_DIR}/_generated/compiled_package.cpp")
set_source_files_properties(${_genFile} PROPERTIES GENERATED TRUE )
list (APPEND sources ${_genFile})

list (APPEND projectFiles ${headers} ${sources} ${c_sources})

# setup
#########
target_definitions()

add_definitions(-D_CONSOLE)
if (MSVC)
	add_definitions(-DWinmain)
endif(MSVC)

# executable and dependancies
message(STATUS "Adding target: EtEngineDemo")
add_executable(EtEngineDemo ${projectFiles})
targetCompileOptions(EtEngineDemo)

# directory stuff
assign_source_group(${projectFiles})
outputDirectories(EtEngineDemo "")

add_dependencies(EtEngineDemo cook-compiled-resources)

# linking
target_link_libraries (EtEngineDemo EtEngine)
dependancyLinks(EtEngineDemo)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (EtEngineDemo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

installDlls(EtEngineDemo "")
installConfig(EtEngineDemo)
installCookResources(EtEngineDemo)