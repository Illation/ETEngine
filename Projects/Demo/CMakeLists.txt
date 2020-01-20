

#####################
# Project CMake file
#####################


# CMake version
cmake_minimum_required (VERSION 3.15.0)

# Project and engine dir
set(ENGINE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine" CACHE STRING "Path to engine root")
get_filename_component(ENGINE_DIRECTORY_ABS "${ENGINE_DIRECTORY}" ABSOLUTE CACHE)
get_filename_component(ENGINE_REPO_DIR "${ENGINE_DIRECTORY}/.." ABSOLUTE CACHE)
get_filename_component(PROJECT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE CACHE)

message(STATUS "//////////////////////////////////////////////////////////////////////////////////////////")
message(STATUS "Engine path: ${ENGINE_DIRECTORY_ABS}")
message(STATUS "Project path: ${PROJECT_DIRECTORY}")
message(STATUS "//////////////////////////////////////////////////////////////////////////////////////////")

# Continuous integration doesn't need to build all configurations for libraries
option(ETE_SINGLE_CONFIG "Build libraries for a single configuration" OFF)
set(ETE_BUILD_LIB_CONFIG "Debug" CACHE STRING "Which configuration to build the library for in case of a single configuration build")

if(ETE_SINGLE_CONFIG)
	message(STATUS "Building libraries only for ${ETE_BUILD_LIB_CONFIG} !")
endif()

# utility functions
message(STATUS "Including function Library ...")
include(${ENGINE_DIRECTORY}/scripts/utils.cmake)

# custom config types
message(STATUS "Setting up configurations ...")
setupConfigurations()

# solution
##########

message(STATUS "Creating Project ...")
project (EtEngineDemo C CXX) # C++
set(CMAKE_CXX_STANDARD 14)  # C++ 14

# run the engine CMake script and then the project
###################################################

createProjectGeneral()

# ensure we can mount the external engine folder
set(_engineMountDir "${PROJECT_DIRECTORY}/build/Engine")

message(STATUS "Adding Engine targets ...")
add_subdirectory (${ENGINE_DIRECTORY} ${_engineMountDir}) # external
message(STATUS "Adding Project targets ...")
add_subdirectory (source/Common)
add_subdirectory (source/Editor)
add_subdirectory (source/Runtime)

# make the Demo the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EtEngineDemo)

message(STATUS "Project generation complete!")