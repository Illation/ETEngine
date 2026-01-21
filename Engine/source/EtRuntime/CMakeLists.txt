

##################
# Runtime Library
##################


# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers})
list (APPEND projectFiles ${sources})

precompiled_headers("${sources}")
target_definitions()

# library and dependancies
message(STATUS "Adding target: EtRuntime")
add_library(EtRuntime ${sources} ${headers})
targetCompileOptions(EtRuntime)

assign_source_group(${projectFiles})
assignIdeFolder(EtRuntime Engine)

libOutputDirectories(EtRuntime)

target_link_libraries (EtRuntime 
    PUBLIC EtApplication EtFramework EtBuild
    PRIVATE engine_runtime_full_policy)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")