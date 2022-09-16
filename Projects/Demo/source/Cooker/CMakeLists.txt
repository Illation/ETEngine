

#####################
# Project Cooker Exe
#####################


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
message(STATUS "Adding target: ProjectCooker")
add_executable(ProjectCooker ${projectFiles})
targetCompileOptions(ProjectCooker)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(ProjectCooker Project/ContentPipeline)
outputDirectories(ProjectCooker "")

addToolDependency(ProjectCooker EtTraceServer)

# linking
target_link_libraries (ProjectCooker EtCooker DemoPipeline)
dependancyLinks(ProjectCooker)
engineLinks(ProjectCooker)
cookerLinks(ProjectCooker)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (ProjectCooker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

copyDllCommand(ProjectCooker)