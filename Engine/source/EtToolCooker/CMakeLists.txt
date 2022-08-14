

##################
# Tool Cooker Exe
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
message(STATUS "Adding target: EtToolCooker")
add_executable(EtToolCooker ${projectFiles})
targetCompileOptions(EtToolCooker)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtToolCooker Engine/Tools)
outputDirectoriesTools(EtToolCooker "")

# linking
target_link_libraries (EtToolCooker EtCooker EtPipeline_GUI)
dependancyLinks(EtToolCooker)
cookerLinks(EtToolCooker)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")

copyDllCommand(EtToolCooker)
copyToolCommand(EtToolCooker EtToolCooker)