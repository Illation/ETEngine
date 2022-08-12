

###############
# Pipeline GUI
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
message(STATUS "Adding target: EtPipeline_GUI")
add_library(EtPipeline_GUI ${projectFiles})
targetCompileOptions(EtPipeline_GUI)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtPipeline_GUI Engine/Pipeline)
libOutputDirectories(EtPipeline_GUI) 

# linking
target_link_libraries (EtPipeline_GUI EtGUI EtPipeline_RHI)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")