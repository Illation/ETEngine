

#########################
# Main Engine CMake file
#########################

# Generate a git-describe version string from Git repository tags
find_package(Git QUIET)
if(GIT_FOUND)
	if(GIT_EXECUTABLE AND NOT DEFINED ET_ENGINE_VERSION)
		execute_process(
			COMMAND ${GIT_EXECUTABLE} describe --tags --match "v*"
			WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
			OUTPUT_VARIABLE GIT_DESCRIBE_VERSION
			RESULT_VARIABLE GIT_DESCRIBE_ERROR_CODE
			OUTPUT_STRIP_TRAILING_WHITESPACE
			)

		if(NOT GIT_DESCRIBE_ERROR_CODE)
			set(ET_ENGINE_VERSION ${GIT_DESCRIBE_VERSION})
		endif()
	endif()
endif()

if(NOT DEFINED ET_ENGINE_VERSION)
	set(ET_ENGINE_VERSION v0.0.0-unknown)
	message(WARNING "Failed to determine ET_ENGINE_VERSION from repository tags. Using default version \"${ET_ENGINE_VERSION}\".")
endif()

message(STATUS "/////////////////////////////////////////////////")
message(STATUS "Engine version: ${ET_ENGINE_VERSION}")
message(STATUS "/////////////////////////////////////////////////")


# general library that has the build files, easy way to make sure we can see the build files in the IDE, shouldn't actually be built
#####################################

file(GLOB generalFiles ${CMAKE_CURRENT_SOURCE_DIR}/../*)
	
file(GLOB_RECURSE engineFiles
	${CMAKE_CURRENT_SOURCE_DIR}/scripts/*
	${CMAKE_CURRENT_SOURCE_DIR}/resources/*
	${CMAKE_CURRENT_SOURCE_DIR}/config/* )

message(STATUS "Adding target: General")
add_custom_target(General ALL SOURCES ${generalFiles} ${engineFiles})
assign_source_group(${engineFiles})
assignIdeFolder(General Engine/General)


# all other targets are under source and resources
##################################################

message(STATUS "Adding third_party targets ...")
add_subdirectory (third_party)
message(STATUS "Adding resource targets ...")
add_subdirectory (resources)
message(STATUS "Adding config targets ...")
add_subdirectory (config)
message(STATUS "Adding source targets ...")
add_subdirectory (source)
message(STATUS "Adding unit_test targets ...")
add_subdirectory (unit_tests)