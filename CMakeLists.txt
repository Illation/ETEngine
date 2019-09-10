

#####################
# Main CMake file
#####################


# CMake version
cmake_minimum_required (VERSION 3.10.0)

# utility functions
include(build/utils.cmake)

# custom config types
setupConfigurations()

# solution
##########

project (EtEngine C CXX) # C++
set(CMAKE_CXX_STANDARD 14)  # C++ 14


# general library that has the build files, easy way to make sure we can see the build files in the IDE, shouldn't actually be built
#####################################

file(GLOB_RECURSE generalFiles
	${CMAKE_CURRENT_SOURCE_DIR}/resources/*.json 
	${CMAKE_CURRENT_SOURCE_DIR}/resources/*.glsl  
	${CMAKE_CURRENT_SOURCE_DIR}/build/*.bat
	${CMAKE_CURRENT_SOURCE_DIR}/build/*.lua
	${CMAKE_CURRENT_SOURCE_DIR}/build/*.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/build/.vahashtags # visual assist
	${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/*.md
	${CMAKE_CURRENT_SOURCE_DIR}/appveyor.yml
	${CMAKE_CURRENT_SOURCE_DIR}/.gitignore
	${CMAKE_CURRENT_SOURCE_DIR}/.gitattributes )

add_library(General ${generalFiles})
set_target_properties(General PROPERTIES LINKER_LANGUAGE NONE) # we don't build this library
assign_source_group(${generalFiles})

# all other targets are under source and resources
##################################################

add_subdirectory (third_party)
add_subdirectory (resources)
add_subdirectory (source)

# make the Demo the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Demo)