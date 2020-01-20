

#######################
# Reflection with RTTR
#######################


# set up vars
#############

set(_buildDir )
getRttrBuildDir(_buildDir)

# if we don't have the libaries files in the place we expect, build the library
###############################################################################

if(NOT EXISTS "${_buildDir}/")

	set(_generator "${CMAKE_GENERATOR}")	
	if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 16 2019")
		set(_architecture " -A ${CMAKE_VS_PLATFORM_NAME}")
	endif()

	set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/rttr")

	if(ETE_SINGLE_CONFIG)
		list (APPEND _configs "${ETE_BUILD_LIB_CONFIG}")
	else()
		list (APPEND _configs "Debug" "Release")
	endif()

    message(STATUS "===================================================")
    message(STATUS "Building RTTR for \"${_generator}\"${_architecture}")
    message(STATUS "===================================================")

	# generate project files
    message(STATUS "Generating project files")
    message(STATUS "------------------------")
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${_generator}" ${_architecture} -DBUILD_BENCHMARKS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_DOCUMENTATION=OFF -DBUILD_UNIT_TESTS=OFF -DBUILD_RTTR_DYNAMIC=OFF -DBUILD_STATIC=ON -H. -B${_buildDir}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _genProjectFiles)
    if(NOT _genProjectFiles EQUAL "0")
        message(FATAL_ERROR "Failed to generate RTTR project files for ${_generator}${_architecture}")
    endif()
	
	# build the libs we need
	foreach(_config ${_configs})

		message(STATUS "Building RTTR for ${_config}")
		message(STATUS "-------------------------------------------")
		
		execute_process(COMMAND ${CMAKE_COMMAND} --build ${_buildDir} --config ${_config}
						WORKING_DIRECTORY ${_modDir}/
						RESULT_VARIABLE _buildTarget)
		if(NOT _buildTarget EQUAL "0")
			message(FATAL_ERROR "Failed to build RTTR for ${_config}")
		endif()

		message(STATUS "Installing RTTR for ${_config}")
		message(STATUS "-----------------------------------")
		
		execute_process(COMMAND ${CMAKE_COMMAND} --build ${_buildDir} --target install --config ${_config}
						WORKING_DIRECTORY ${_modDir}/
						RESULT_VARIABLE _install)
		if(NOT _install EQUAL "0")
			message(FATAL_ERROR "Failed to install RTTR")
		endif()

	endforeach()

    message(STATUS "=======================")
    message(STATUS "Finished building RTTR")
    message(STATUS "=======================")

else()

	message(STATUS "Library already built: RTTR")

endif()
