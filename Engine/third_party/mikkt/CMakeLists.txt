

####################################
# Tangent Space Calculation Library
####################################


file(GLOB_RECURSE projectFiles ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)


message(STATUS "Adding target: MikkTSpace")
add_library(MikkTSpace ${projectFiles})

assign_source_group(${projectFiles})
assignIdeFolder(MikkTSpace Engine/3rdParty)
libOutputDirectories(MikkTSpace)
