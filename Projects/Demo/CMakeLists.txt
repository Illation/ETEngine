

#####################
# Project CMake file
#####################


# CMake version
cmake_minimum_required (VERSION 3.15.0)

set(ENGINE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../Engine" CACHE STRING "Path to engine root")

# utility functions
message(STATUS "Including function Library ...")
include(${ENGINE_DIRECTORY}/scripts/utils.cmake)

# Project and engine dir
inferPaths()

setGenOptions()

# custom config types
message(STATUS "Setting up configurations ...")
setupConfigurations()

# solution
##########

message(STATUS "Creating Project ...")
project (EtEngineDemo C CXX) # C++
set(CMAKE_CXX_STANDARD 14)  # C++ 14

createCompilePolicies()

# run the engine CMake script and then the project
###################################################

createProjectGeneral()

# ensure we can mount the external engine folder
set(_engineMountDir "${PROJECT_DIRECTORY}/build/Engine")

message(STATUS "Adding Engine targets ...")
add_subdirectory (${ENGINE_DIRECTORY} ${_engineMountDir}) # external
message(STATUS "Adding Project targets ...")
add_subdirectory (source/Common)
add_subdirectory (source/Pipeline)
add_subdirectory (source/Editor)
add_subdirectory (source/Cooker)
add_subdirectory (source/Runtime)

# make the Demo the startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT EtEngineDemo)

message(STATUS "Project generation complete!")