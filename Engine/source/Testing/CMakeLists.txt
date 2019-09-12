

##############
# Unit Tests
##############


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.glsl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources} ${c_sources})

# setup
#########
target_definitions()

add_definitions(-D_CONSOLE)
# executable and dependancies
add_executable(Testing ${projectFiles})
targetCompileOptions(Testing)

# directory stuff
assign_source_group(${projectFiles})
outputDirectories(Testing)

# linking
target_link_libraries (Testing EtEngine)
dependancyLinks(Testing)

# library includes
libIncludeDirs()
include_directories("${ENGINE_DIRECTORY_ABS}/third_party/Catch2/single_include/")	

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (Testing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

installDlls(Testing)
installResources(Testing)