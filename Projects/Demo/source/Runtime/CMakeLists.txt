

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

# executable and dependancies
message(STATUS "Adding target: EtEngineDemo")
if (MSVC)
	add_executable(EtEngineDemo WIN32 ${projectFiles})
else()
	add_executable(EtEngineDemo ${projectFiles})
endif()

targetCompileOptions(EtEngineDemo)
addDebugVisualizers(EtEngineDemo)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtEngineDemo Project)
outputDirectories(EtEngineDemo "")

add_dependencies(EtEngineDemo cook-compiled-resources)
addToolDependency(EtEngineDemo EtTraceServer)

# linking
target_link_libraries (EtEngineDemo DemoCommon EtRuntime)
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