

##############
# Demo Editor
##############


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE c_sources ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

list (APPEND projectFiles ${headers} ${sources} ${c_sources})

# setup
#########
target_definitions()
add_definitions(-D_CONSOLE)

# executable and dependancies
add_executable(DemoEditor ${projectFiles})
targetCompileOptions(DemoEditor)

# directory stuff
assign_source_group(${projectFiles})
outputDirectories(DemoEditor "/bin") 

add_dependencies(DemoEditor compile-resources-editor)
add_dependencies(DemoEditor compile-schemas-editor)

# linking
message(STATUS "Adding target: DemoEditor")
target_link_libraries (DemoEditor EtEngine)
dependancyLinks(DemoEditor) 
editorLinks(DemoEditor)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (DemoEditor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

installDlls(DemoEditor "/bin")
installEditorDlls(DemoEditor)
installEditorResources(DemoEditor)