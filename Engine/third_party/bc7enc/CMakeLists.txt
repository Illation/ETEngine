

####################################
# BC7 ENC - Texture compression
####################################


set (sources 
	${CMAKE_CURRENT_SOURCE_DIR}/bc7enc/bc7enc.c
	${CMAKE_CURRENT_SOURCE_DIR}/bc7enc/bc7decomp.cpp )
	
set (headers 
	${CMAKE_CURRENT_SOURCE_DIR}/bc7enc/bc7enc.h
	${CMAKE_CURRENT_SOURCE_DIR}/bc7enc/bc7decomp.h 
	${CMAKE_CURRENT_SOURCE_DIR}/bc7enc/rgbcx.h
	${CMAKE_CURRENT_SOURCE_DIR}/bc7enc/rgbcx_table4.h )

list (APPEND projectFiles ${sources} ${headers})

message(STATUS "Adding target: bc7enc")
add_library(bc7enc ${projectFiles})
targetCompileOptions(bc7enc)

assign_source_group(${projectFiles})
assignIdeFolder(bc7enc Engine/3rdParty)
libOutputDirectories(bc7enc)
