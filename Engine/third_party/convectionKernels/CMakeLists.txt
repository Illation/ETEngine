

###########################################
# Convection Kernels - Texture compression
###########################################


set (sources ${CMAKE_CURRENT_SOURCE_DIR}/ConvectionKernels/ConvectionKernels_SingleFile.cpp )
set (headers ${CMAKE_CURRENT_SOURCE_DIR}/ConvectionKernels/ConvectionKernels.h)

list (APPEND projectFiles ${sources} ${headers})

message(STATUS "Adding target: ConvectionKernels")
add_library(ConvectionKernels ${projectFiles})
targetCompileOptions(ConvectionKernels)
if(MSVC)
	target_compile_options(ConvectionKernels PRIVATE "/DCVTT_SINGLE_FILE")
else()
	target_compile_options(ConvectionKernels PRIVATE "-DCVTT_SINGLE_FILE")
endif()

assign_source_group(${projectFiles})
assignIdeFolder(ConvectionKernels Engine/3rdParty)
libOutputDirectories(ConvectionKernels)

target_include_directories (ConvectionKernels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
