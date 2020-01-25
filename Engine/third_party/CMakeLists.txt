

##############
# Submodules
##############


# Download them
#---------------

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${ENGINE_REPO_DIR}/.git")
# Update submodules as needed
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()


# Add them
#---------------

add_subdirectory (glfw)
add_subdirectory (assimp)
add_subdirectory (bullet)
add_subdirectory (rttr)
add_subdirectory (openal)
if(WIN32)
	add_subdirectory (vcpkg)
endif()
add_subdirectory (stb)
add_subdirectory (mikkt)
add_subdirectory (glad)