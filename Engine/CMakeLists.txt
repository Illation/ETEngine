

#########################
# Main Engine CMake file
#########################


# general library that has the build files, easy way to make sure we can see the build files in the IDE, shouldn't actually be built
#####################################

file(GLOB generalFiles ${CMAKE_CURRENT_SOURCE_DIR}/../*)
	
file(GLOB_RECURSE engineFiles
	${CMAKE_CURRENT_SOURCE_DIR}/scripts/*
	${CMAKE_CURRENT_SOURCE_DIR}/resources/*
	${CMAKE_CURRENT_SOURCE_DIR}/config/* )

message(STATUS "Adding target: General")
add_library(General ${generalFiles} ${engineFiles})
set_target_properties(General PROPERTIES LINKER_LANGUAGE NONE) # we don't build this library
assign_source_group(${engineFiles})


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