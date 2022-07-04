

#################
# GUI with RmlUi
#################


# set up vars
#############

set(_buildDir )
getRmlUiBuildDir(_buildDir)

# if we don't have the libaries files in the place we expect, build the library
###############################################################################

if(NOT EXISTS "${_buildDir}/")

	set(_generator "${CMAKE_GENERATOR}")	
	if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 16 2019")
		set(_architecture " -A ${CMAKE_VS_PLATFORM_NAME}")
	endif()

	set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/RmlUi")

	if(ETE_SINGLE_CONFIG)
		list (APPEND _configs "${ETE_BUILD_LIB_CONFIG}")
	else()
		list (APPEND _configs "Debug" "Release")
	endif()

    message(STATUS "===================================================")
    message(STATUS "Building RmlUi for \"${_generator}\"${_architecture}")
    message(STATUS "===================================================")
	
	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)

	# generate project files
    message(STATUS "Generating project files")
    message(STATUS "------------------------")
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${_generator}" ${_architecture} -DNO_FONT_INTERFACE_DEFAULT=ON  -DBUILD_SHARED_LIBS=OFF -H. -B${_buildDir}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _genProjectFiles)
    if(NOT _genProjectFiles EQUAL "0")
        message(FATAL_ERROR "Failed to generate RmlUi project files for ${_generator}${_architecture}")
    endif()
	
	# build the libs we need
	foreach(_config ${_configs})

		message(STATUS "Building and Installing RmlUi for ${_config}")
		message(STATUS "-----------------------------------------------")
		
		execute_process(COMMAND ${CMAKE_COMMAND} --build ${_buildDir} --config ${_config}
						WORKING_DIRECTORY ${_modDir}/
						RESULT_VARIABLE _install)
		if(NOT _install EQUAL "0")
			message(FATAL_ERROR "Failed to install RmlUi")
		endif()

	endforeach()

    message(STATUS "=======================")
    message(STATUS "Finished building RmlUi")
    message(STATUS "=======================")

else()

	message(STATUS "Library already built: RmlUi")

endif()
