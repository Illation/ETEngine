

##############
# Demo
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
if (MSVC)
	add_definitions(-DWinmain)
endif(MSVC)

# executable and dependancies
add_executable(Demo WIN32 ${projectFiles})

# directory stuff
assign_source_group(${projectFiles})
outputDirectories(Demo)

#add_dependencies(Synthesizer run-cooker)

# linking
target_link_libraries (Demo EtEngine)
dependancyLinks(Demo TRUE)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (Demo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

installDlls(Demo)
installResources(Demo)