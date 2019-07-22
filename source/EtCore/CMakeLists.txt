

################
# Core Library
################


# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.inl ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)

file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers})
list (APPEND projectFiles ${sources})

precompiled_headers(${sources})
target_definitions()

# library and dependancies
add_library(EtCore ${sources} ${headers})

assign_source_group(${projectFiles})

libOutputDirectories(EtCore)

target_link_libraries (EtCore EtMath)

# library includes
libIncludeDirs()
#include_directories("../../dependancies/include/gtkmm/")

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (EtCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})