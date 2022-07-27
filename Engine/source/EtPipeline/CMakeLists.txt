

###########
# Pipeline
###########


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources})

# setup
#########
target_definitions()
add_definitions(-D_CONSOLE)

# library
message(STATUS "Adding target: EtPipeline")
add_library(EtPipeline ${projectFiles})
targetCompileOptions(EtPipeline)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtPipeline Engine/Tools)
libOutputDirectories(EtPipeline) 

# linking
target_link_libraries (EtPipeline EtFramework EtBuild bc7enc ConvectionKernels MikkTSpace)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")