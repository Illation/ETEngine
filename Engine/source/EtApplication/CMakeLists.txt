

######################
# Application Library
######################


# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers})
list (APPEND projectFiles ${sources})

precompiled_headers("${sources}")
target_definitions()

# library and dependancies
message(STATUS "Adding target: EtApplication")
add_library(EtApplication ${sources} ${headers})
targetCompileOptions(EtApplication)

assign_source_group(${projectFiles})
assignIdeFolder(EtApplication Engine)

libOutputDirectories(EtApplication)

target_link_libraries (EtApplication EtGUI glad)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")