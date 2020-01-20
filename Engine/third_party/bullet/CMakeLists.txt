

#################
# Bullet Physics
#################


# set up vars
#############

set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/bullet3")

set(_buildDir )
getBulletBuildDir(_buildDir)

if(ETE_SINGLE_CONFIG)
	list (APPEND _configs "${ETE_BUILD_LIB_CONFIG}")
else()
	list (APPEND _configs "Debug" "Release")
endif()
list (APPEND _targets "BulletCollision" "BulletDynamics" "LinearMath")

# if we don't have the libaries files in the place we expect, build the library
###############################################################################

if(NOT EXISTS "${_buildDir}/")

	set(_generator "${CMAKE_GENERATOR}")	
	if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 16 2019")
		set(_architecture " -A ${CMAKE_VS_PLATFORM_NAME}")
	endif()

    message(STATUS "=============================================================")
    message(STATUS "Building Bullet Physics for \"${_generator}\"${_architecture}")
    message(STATUS "=============================================================")

	# generate project files
    message(STATUS "Generating project files")
    message(STATUS "------------------------")
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${_generator}" ${_architecture} -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON -H. -B${_buildDir}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _genProjectFiles)
    if(NOT _genProjectFiles EQUAL "0")
        message(FATAL_ERROR "Failed to generate bullet project files for ${_generator}${_architecture}")
    endif()
	
	# build the libs we need
	foreach(_config ${_configs})
		foreach(_target ${_targets})
			message(STATUS "Building library ${_target} on ${_config}")
			message(STATUS "-------------------------------------------")
			
			execute_process(COMMAND ${CMAKE_COMMAND} --build ${_buildDir} --target ${_target} --config ${_config}
							WORKING_DIRECTORY ${_modDir}/
							RESULT_VARIABLE _buildTarget)
			if(NOT _buildTarget EQUAL "0")
				message(FATAL_ERROR "Failed to build target ${_target} on ${_config}")
			endif()

		endforeach()
	endforeach()

    message(STATUS "=================================")
    message(STATUS "Finished building Bullet Physics")
    message(STATUS "=================================")

else()

	message(STATUS "Library already built: Bullet Physics")

endif()
