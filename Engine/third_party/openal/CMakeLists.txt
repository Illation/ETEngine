

#########################
# OpenAL Soft - 3D Sound
#########################


# set up vars
#############

set(_buildDir )
getOpenAlBuildDir(_buildDir)

# if we don't have the libaries files in the place we expect, build the library
###############################################################################

if(NOT EXISTS "${_buildDir}/")

	set(_generator "${CMAKE_GENERATOR}")	
	if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 16 2019")
		set(_architecture " -A ${CMAKE_VS_PLATFORM_NAME}")
	endif()

	set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/openal-soft")

	if(ETE_SINGLE_CONFIG)
		list (APPEND _configs "${ETE_BUILD_LIB_CONFIG}")
	else()
		list (APPEND _configs "Debug" "Release")
	endif()

    message(STATUS "=====================================================")
    message(STATUS "Building OpenAL for \"${_generator}\"${_architecture}")
    message(STATUS "=====================================================")

	# generate project files
    message(STATUS "Generating project files")
    message(STATUS "------------------------")
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${_generator}" ${_architecture} -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_TESTS=OFF -H. -B${_buildDir}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _genProjectFiles)
    if(NOT _genProjectFiles EQUAL "0")
        message(FATAL_ERROR "Failed to generate OpenAL project files for ${_generator}${_architecture}")
    endif()
	
	# build the libs we need
	foreach(_config ${_configs})

		message(STATUS "Building OpenAL for ${_config}")
		message(STATUS "--------------------------------")
		
		execute_process(COMMAND ${CMAKE_COMMAND} --build ${_buildDir} --config ${_config}
						WORKING_DIRECTORY ${_modDir}/
						RESULT_VARIABLE _buildTarget)
		if(NOT _buildTarget EQUAL "0")
			message(FATAL_ERROR "Failed to build OpenAL for ${_config}")
		endif()

	endforeach()

    message(STATUS "=========================")
    message(STATUS "Finished building OpenAL")
    message(STATUS "=========================")

else()

	message(STATUS "Library already built: OpenAL")

endif()
