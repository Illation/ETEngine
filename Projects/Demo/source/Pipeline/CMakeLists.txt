

################
# Demo Pipeline
################


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources})

# setup
#########
target_definitions()

# executable and dependancies
message(STATUS "Adding target: DemoPipeline")
add_library(DemoPipeline ${projectFiles})
targetCompileOptions(DemoPipeline)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(DemoPipeline Project/ContentPipeline)

libOutputDirectories(DemoPipeline)

# linking
target_link_libraries (DemoPipeline DemoCommon)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (DemoPipeline PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})