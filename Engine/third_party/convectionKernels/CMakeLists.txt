

###########################################
# Convection Kernels - Texture compression
###########################################


set (sources ${CMAKE_CURRENT_SOURCE_DIR}/convectionKernelsImpl.cpp )
set (headers ${CMAKE_CURRENT_SOURCE_DIR}/ConvectionKernels/ConvectionKernels.h)

list (APPEND projectFiles ${sources} ${headers})

message(STATUS "Adding target: ConvectionKernels")
add_library(ConvectionKernels ${projectFiles})

assign_source_group(${projectFiles})
assignIdeFolder(ConvectionKernels Engine/3rdParty)
libOutputDirectories(ConvectionKernels)

target_include_directories (ConvectionKernels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
