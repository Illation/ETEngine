

#####################
# Engine Library
#####################


# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.inl )
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE c_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

list (APPEND projectFiles ${headers} ${sources} ${c_sources})

precompiled_headers(${sources})
target_definitions()

# library and dependancies
add_library(EtEngine ${projectFiles})
targetCompileOptions(EtEngine)

assign_source_group(${projectFiles})
libOutputDirectories(EtEngine)

target_link_libraries (EtEngine ThirdParty EtMath EtCore)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (EtEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})