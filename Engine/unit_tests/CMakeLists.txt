

##############
# Unit Tests
##############


# files
###########
file(GLOB_RECURSE headers ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

list (APPEND projectFiles ${headers} ${sources} ${c_sources})

# setup
#########
target_definitions()

add_definitions(-D_CONSOLE)
# executable and dependancies
message(STATUS "Adding target: unit_tests")
add_executable(unit_tests ${projectFiles})
targetCompileOptions(unit_tests)

# directory stuff
assign_source_group(${projectFiles})
assignIdeFolder(unit_tests Engine/UnitTests)
outputDirectories(unit_tests "")

# linking
target_link_libraries (unit_tests EtEngine)
dependancyLinks(unit_tests)

# library includes
libIncludeDirs()
include_directories("${ENGINE_DIRECTORY_ABS}/third_party/Catch2/single_include/")	

# general include dirs
include_directories("${ENGINE_DIRECTORY_ABS}/source")
target_include_directories (unit_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

installDlls(unit_tests "")