

###################
# Trace Server Exe
###################


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

set (_genDir "${CMAKE_CURRENT_SOURCE_DIR}/_generated/")
set(_genFile "${_genDir}compiled_package.cpp")
set_source_files_properties(${_genFile} PROPERTIES GENERATED TRUE )
list (APPEND sources ${_genFile})

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
outputDirectoriesTools(EtTraceServer)

# linking
target_link_libraries(EtTraceServer EtApplication)
dependancyLinks(EtTraceServer)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")

cookToolData(EtTraceServer "${ENGINE_DIRECTORY_ABS}/resources/asset_database_traceserver.json" "${_genDir}")
set(_tool_cook_target_name)
getToolCookTargetName(EtTraceServer _tool_cook_target_name)

add_dependencies(EtTraceServer ${_tool_cook_target_name})

copyDllCommand(EtTraceServer)

copyToolCommand(EtTraceServer EtTraceServer)