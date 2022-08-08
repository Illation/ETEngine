

#########
# Editor
#########


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE c_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

list (APPEND projectFiles ${headers} ${sources} ${c_sources})

# setup
#########
precompiled_headers("${sources}")
target_definitions()
add_definitions(-D_CONSOLE)

# library
message(STATUS "Adding target: EtEditor")
add_library(EtEditor ${projectFiles})
targetCompileOptions(EtEditor)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtEditor Engine/Tools)
libOutputDirectories(EtEditor) 

# linking
target_link_libraries (EtEditor EtPipeline)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")