

#########################
# Main Engine CMake file
#########################


# general library that has the build files, easy way to make sure we can see the build files in the IDE, shouldn't actually be built
#####################################

file(GLOB_RECURSE generalFiles
	${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.cmake
	${CMAKE_CURRENT_SOURCE_DIR}/../CMakeLists.txt
	${CMAKE_CURRENT_SOURCE_DIR}/../*.md
	${CMAKE_CURRENT_SOURCE_DIR}/../appveyor.yml
	${CMAKE_CURRENT_SOURCE_DIR}/../.gitignore
	${CMAKE_CURRENT_SOURCE_DIR}/../.gitattributes )

add_library(General ${generalFiles})
set_target_properties(General PROPERTIES LINKER_LANGUAGE NONE) # we don't build this library
assign_source_group(${generalFiles})

# all other targets are under source and resources
##################################################

add_subdirectory (third_party)
add_subdirectory (resources)
add_subdirectory (config)
add_subdirectory (source)