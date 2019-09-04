

####################################
# Tangent Space Calculation Library
####################################


file(GLOB_RECURSE projectFiles ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(MikkTSpace ${projectFiles})

assign_source_group(${projectFiles})
libOutputDirectories(MikkTSpace)
