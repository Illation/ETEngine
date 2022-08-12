

#####################
# Pipeline Framework
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
message(STATUS "Adding target: EtPipeline_Framework")
add_library(EtPipeline_Framework ${projectFiles})
targetCompileOptions(EtPipeline_Framework)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtPipeline_Framework Engine/Pipeline)
libOutputDirectories(EtPipeline_Framework) 

# linking
target_link_libraries (EtPipeline_Framework EtFramework EtPipeline_Rendering EtPipeline_GUI)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")