

#################
# Bullet Physics
#################


# set up vars
#############

if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
	set(_p "x64")	
	set(_generator "Visual Studio 15 2017 Win64")	
else() # 32 bit
	set(_p "x32")	
	set(_generator "Visual Studio 15 2017")	
endif()

set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/bullet3")
set(_buildDir "../build/${_p}")

list (APPEND _configs "Debug" "Release")
list (APPEND _targets "BulletCollision" "BulletDynamics" "LinearMath")

set(_installDir "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(_outIncDir "${_installDir}/include/bullet")
set(_outLibDir "${_installDir}/${_p}/bullet")

# if we don't have the libaries files in the place we expect, build the library
###############################################################################

if((NOT EXISTS "${_outIncDir}/") OR (NOT EXISTS "${_outLibDir}/"))

    message(STATUS "=========================================")
    message(STATUS "Building Bullet Physics for ${_generator}")
    message(STATUS "=========================================")

	# generate project files
    message(STATUS "Generating project files")
    message(STATUS "------------------------")
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${_generator}" -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON -H. -B${_buildDir}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _genProjectFiles)
    if(NOT _genProjectFiles EQUAL "0")
        message(FATAL_ERROR "Failed to generate bullet project files for ${_generator}")
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

	# copy includes
    message(STATUS "Copying include files")
    message(STATUS "---------------------")

	file(GLOB_RECURSE _includeFiles ${_modDir}/src/*.h ${_modDir}/src/*.hpp)
	foreach(_includeFile ${_includeFiles})
		file(RELATIVE_PATH _relPath ${_modDir}/src ${_includeFile})
		get_filename_component(_relDir ${_relPath} DIRECTORY)
		file(COPY ${_includeFile} DESTINATION ${_outIncDir}/${_relDir})
	endforeach()

	# copy lib files
    message(STATUS "Copying library files")
    message(STATUS "---------------------")

	file(GLOB_RECURSE _libFiles ${_modDir}/${_buildDir}/lib/*)
	foreach(_libFile ${_libFiles})
		file(RELATIVE_PATH _relPath ${_modDir}/${_buildDir}/lib ${_libFile})
		get_filename_component(_relDir ${_relPath} DIRECTORY)
		file(COPY ${_libFile} DESTINATION ${_outLibDir}/${_relDir})
	endforeach()

    message(STATUS "=================================")
    message(STATUS "Finished building bullet Physics")
    message(STATUS "=================================")
endif()
