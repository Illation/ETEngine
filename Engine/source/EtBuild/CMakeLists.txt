

#############################################
# Making Build arguments available in source
#############################################

# generate the version source file
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/EngineVersion.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/EngineVersion.cpp)

# generate the development paths source file
set(BUILD_TRACESERVER_NAME EtTraceServer)
set(BUILD_TRACESERVER_DIR)
getToolCopyDir(EtTraceServer BUILD_TRACESERVER_DIR)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/DevelopmentPaths.cpp.in ${CMAKE_CURRENT_SOURCE_DIR}/DevelopmentPaths.cpp)

# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp.in)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers})
list (APPEND projectFiles ${sources})

target_definitions()

# header only library builds with one source file that includes all headers
message(STATUS "Adding target: EtBuild")
add_library(EtBuild ${projectFiles})
targetCompileOptions(EtBuild)

assign_source_group(${projectFiles})
assignIdeFolder(EtBuild Engine)
libOutputDirectories(EtBuild)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (EtBuild PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})