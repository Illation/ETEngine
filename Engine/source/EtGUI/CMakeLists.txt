

#########################
# User Interface Library
#########################


# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.inl )
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources})

precompiled_headers("${sources}")
target_definitions()

# library and dependancies
message(STATUS "Adding target: EtGUI")
add_library(EtGUI ${projectFiles})
targetCompileOptions(EtGUI)

assign_source_group(${projectFiles})
assignIdeFolder(EtGUI Engine)
libOutputDirectories(EtGUI)

target_link_libraries (EtGUI EtRendering EtMath EtCore imgui)

# library includes
libIncludeDirs()

# general include dirs
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
target_include_directories (EtGUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})