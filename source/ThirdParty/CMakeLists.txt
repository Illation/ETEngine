

#########################################
# Third party libraries built from source
#########################################


# files in this library
file(GLOB_RECURSE projectFiles ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

target_definitions()

# library and dependancies
add_library(ThirdParty ${projectFiles})

assign_source_group(${projectFiles})
libOutputDirectories(ThirdParty)

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (ThirdParty PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})