

###################
# Trace Server Exe
###################


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources} ${c_sources})

# setup
#########
precompiled_headers("${sources}")
target_definitions()

add_definitions(-D_CONSOLE)
# executable and dependancies
message(STATUS "Adding target: EtTraceServer")
if (MSVC)
	add_executable(EtTraceServer WIN32 ${projectFiles})
else()
	add_executable(EtTraceServer ${projectFiles})
endif()

targetCompileOptions(EtTraceServer)
addDebugVisualizers(EtTraceServer)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(EtTraceServer Engine/Tools)
outputDirectoriesTools(EtTraceServer "")

# linking
target_link_libraries(EtTraceServer EtCore EtBuild)
dependancyLinks(EtTraceServer)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")

copyDllCommand(EtTraceServer)
copyToolCommand(EtTraceServer)