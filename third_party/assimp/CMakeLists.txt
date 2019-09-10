

#############################
# Mesh importing with Assimp
#############################


# set up vars
#############

set(_buildDir )
getAssimpBuildDir(_buildDir)

# if we don't have the libaries files in the place we expect, build the library
###############################################################################

if(NOT EXISTS "${_buildDir}/")

	if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
		set(_generator "Visual Studio 15 2017 Win64")	
	else() # 32 bit
		set(_generator "Visual Studio 15 2017")	
	endif()

	set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/assimp")
	list (APPEND _configs "Debug" "Release")

    message(STATUS "================================")
    message(STATUS "Building Assimp for ${_generator}")
    message(STATUS "================================")

	# generate project files
    message(STATUS "Generating project files")
    message(STATUS "------------------------")
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${_generator}" -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_LIB=ON -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ASSIMP_TOOLS=OFF -DASSIMP_BUILD_TESTS=OFF -H. -B${_buildDir}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _genProjectFiles)
    if(NOT _genProjectFiles EQUAL "0")
        message(FATAL_ERROR "Failed to generate Assimp project files for ${_generator}")
    endif()
	
	# build the libs we need
	foreach(_config ${_configs})

		message(STATUS "Building Assimp for ${_config}")
		message(STATUS "-------------------------------------------")
		
		execute_process(COMMAND ${CMAKE_COMMAND} --build ${_buildDir} --config ${_config}
						WORKING_DIRECTORY ${_modDir}/
						RESULT_VARIABLE _buildTarget)
		if(NOT _buildTarget EQUAL "0")
			message(FATAL_ERROR "Failed to build Assimp for ${_config}")
		endif()

	endforeach()

    message(STATUS "=========================")
    message(STATUS "Finished building Assimp")
    message(STATUS "=========================")

endif()
