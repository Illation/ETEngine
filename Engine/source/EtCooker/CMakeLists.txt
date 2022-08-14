

##################
# Resource Cooker
##################


# files
###########
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

list (APPEND projectFiles ${sources} ${headers})

# setup
#########
target_definitions()
add_definitions(-D_CONSOLE)

# executable and dependancies
message(STATUS "Adding target: EtCooker")
add_library(EtCooker ${projectFiles})
targetCompileOptions(EtCooker)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtCooker Engine/Tools)
libOutputDirectories(EtCooker)

# linking
target_link_libraries (EtCooker EtPipeline EtApplication)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")