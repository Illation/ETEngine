

###############
# Pipeline RHI
###############


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources})

# setup
#########
target_definitions()

# library
message(STATUS "Adding target: EtPipeline_RHI")
add_library(EtPipeline_RHI ${projectFiles})
targetCompileOptions(EtPipeline_RHI)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtPipeline_RHI Engine/Pipeline)
libOutputDirectories(EtPipeline_RHI) 

# linking
target_link_libraries (EtPipeline_RHI EtRHI EtPipeline bc7enc ConvectionKernels)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")