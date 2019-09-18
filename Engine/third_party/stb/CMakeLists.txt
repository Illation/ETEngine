

#################
# STB libraries
#################


file(GLOB projectFiles ${CMAKE_CURRENT_SOURCE_DIR}/stb/*.c 
	${CMAKE_CURRENT_SOURCE_DIR}/stb/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/*.h 
	${CMAKE_CURRENT_SOURCE_DIR}/*.c)

message(STATUS "Adding target: NothingsStb")
add_library(NothingsStb ${projectFiles})

assign_source_group(${projectFiles})
assignIdeFolder(NothingsStb Engine/3rdParty)
libOutputDirectories(NothingsStb)
