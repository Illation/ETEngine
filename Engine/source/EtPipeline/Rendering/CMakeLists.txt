

#####################
# Pipeline Rendering
#####################


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources})

# setup
#########
target_definitions()

# library
message(STATUS "Adding target: EtPipeline_Rendering")
add_library(EtPipeline_Rendering ${projectFiles})
targetCompileOptions(EtPipeline_Rendering)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtPipeline_Rendering Engine/Pipeline)
libOutputDirectories(EtPipeline_Rendering) 

# linking
target_link_libraries (EtPipeline_Rendering EtRendering EtPipeline_RHI)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")