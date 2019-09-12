

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

	if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
		set(_generator "Visual Studio 15 2017 Win64")	
	else() # 32 bit
		set(_generator "Visual Studio 15 2017")	
	endif()

	set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/openal-soft")
	list (APPEND _configs "Debug" "Release")

    message(STATUS "================================")
    message(STATUS "Building OpenAL for ${_generator}")
    message(STATUS "================================")

	# generate project files
    message(STATUS "Generating project files")
    message(STATUS "------------------------")
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${_generator}" -DALSOFT_UTILS=OFF -DALSOFT_EXAMPLES=OFF -DALSOFT_TESTS=OFF -H. -B${_buildDir}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _genProjectFiles)
    if(NOT _genProjectFiles EQUAL "0")
        message(FATAL_ERROR "Failed to generate OpenAL project files for ${_generator}")
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

endif()
