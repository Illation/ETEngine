

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
add_executable(Demo ${projectFiles})
targetCompileOptions(Demo)

# directory stuff
assign_source_group(${projectFiles})
outputDirectories(Demo)

add_dependencies(Demo cook-compiled-resources)

# linking
target_link_libraries (Demo EtEngine)
dependancyLinks(Demo)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (Demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

installDlls(Demo)
installResources(Demo)
installCookResources(Demo)