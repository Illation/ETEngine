

################
# Math Library
################


# files in this library
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.inl)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers})
list (APPEND projectFiles ${sources})

target_definitions()

# header only library builds with one source file that includes all headers
add_library(EtMath ${projectFiles})

assign_source_group(${projectFiles})

# base directory
target_include_directories (EtMath PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

libOutputDirectories(EtMath)