

####################################
# Render Hardware Interface Library
####################################


# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.inl )
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE c_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

list (APPEND projectFiles ${headers} ${sources} ${c_sources})

precompiled_headers("${sources}")
target_definitions()

# library and dependancies
message(STATUS "Adding target: EtRHI")
add_library(EtRHI ${projectFiles})
targetCompileOptions(EtRHI)

assign_source_group(${projectFiles})
assignIdeFolder(EtRHI Engine)
libOutputDirectories(EtRHI)

target_link_libraries (EtRHI NothingsStb EtMath EtCore)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")