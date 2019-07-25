

##################
# Resource Cooker
##################


# files
###########
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${sources})

# setup
#########
target_definitions()

add_definitions(-D_CONSOLE)

add_definitions(-DUNCOOKED_RESOURCES) 

# executable and dependancies
add_executable(EtCooker ${projectFiles})

# directory stuff
assign_source_group(${projectFiles})
outputDirectories(EtCooker)

# linking
target_link_libraries (EtCooker EtEngine)
dependancyLinks(EtCooker FALSE)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (EtCooker PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

installDlls(EtCooker)