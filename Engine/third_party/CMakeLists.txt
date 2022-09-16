

##############
# Submodules
##############


# Download them
#---------------

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${ENGINE_REPO_DIR}/.git")
# Update submodules as needed
    message(STATUS "Submodule update")
	if(ETE_UPDATE_SUBMODULES)
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive --remote ${ETE_UPDATE_SUBMODULE_PATH}
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						RESULT_VARIABLE GIT_SUBMOD_RESULT)
	else()
		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
						WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
						RESULT_VARIABLE GIT_SUBMOD_RESULT)
	endif()
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
endif()


# Add them
#---------------

# prebuilt libraries
add_subdirectory (glfw)
add_subdirectory (bullet)
add_subdirectory (rttr)
add_subdirectory (openal)
add_subdirectory (lunasvg)
add_subdirectory (rmlui)
if(WIN32)
	add_subdirectory (vcpkg)
endif()

# generated libraries
add_subdirectory (glad)

# integrated libraries
add_subdirectory (stb)
add_subdirectory (mikkt)
add_subdirectory (bc7enc)
add_subdirectory (convectionKernels)
add_subdirectory (imgui)