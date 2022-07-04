

####################
# Framework Library
####################


# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.inl )
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources})

precompiled_headers(${sources})
target_definitions()

# library and dependancies
message(STATUS "Adding target: EtFramework")
add_library(EtFramework ${projectFiles})
targetCompileOptions(EtFramework)

assign_source_group(${projectFiles})
assignIdeFolder(EtFramework Engine)
libOutputDirectories(EtFramework)

target_link_libraries (EtFramework NothingsStb EtGUI EtRendering EtMath EtCore)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (EtFramework PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})