
##########################
# Tracy - Frame Profiling
##########################


# build the server application
##############################

# set up vars
set(_serverBuildDir )
getTracyServerBuildDir(_serverBuildDir)

# set(_cacheDir "${ENGINE_DIRECTORY_ABS}/third_party/tracy/cache/${_p}" PARENT_SCOPE)

if (ETE_BUILD_TRACY_SERVER AND (NOT EXISTS "${_serverBuildDir}/"))

	set(_generator "${CMAKE_GENERATOR}")	
	if("${CMAKE_GENERATOR}" MATCHES "Visual Studio 16 2019")
		set(_architecture " -A ${CMAKE_VS_PLATFORM_NAME}")
	endif()

	set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/tracy/profiler")

	list (APPEND _configs "Release")
	# if(ETE_SINGLE_CONFIG)
		# list (APPEND _configs "${ETE_BUILD_LIB_CONFIG}")
	# else()
		# list (APPEND _configs "Debug" "Release")
	# endif()

    message(STATUS "===========================================================")
    message(STATUS "Building Tracy Server for \"${_generator}\"${_architecture}")
    message(STATUS "===========================================================")

	# generate project files
    message(STATUS "Generating project files")
    message(STATUS "------------------------")
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${_generator}" ${_architecture} -H. -B${_serverBuildDir}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _genProjectFiles)
    if(NOT _genProjectFiles EQUAL "0")
        message(FATAL_ERROR "Failed to generate Tracy Profiler project files for ${_generator}${_architecture}")
    endif()
	
	# build the libs we need
	foreach(_config ${_configs})

		message(STATUS "Building Tracy Profiler for ${_config}")
		message(STATUS "--------------------------------------")
		
		execute_process(COMMAND ${CMAKE_COMMAND} --build ${_serverBuildDir} --config ${_config} --parallel
						WORKING_DIRECTORY ${_modDir}/
						RESULT_VARIABLE _buildTarget)
		if(NOT _buildTarget EQUAL "0")
			message(FATAL_ERROR "Failed to build Tracy Profiler for ${_config}")
		endif()

	endforeach()

    message(STATUS "==============================")
    message(STATUS "Finished building Tracy Server")
    message(STATUS "==============================")

else()
	message(STATUS "Library already built: Tracy")
endif()

# integrate the client
######################

set (sources 
	${CMAKE_CURRENT_SOURCE_DIR}/tracy/public/TracyClient.cpp)
	
set (headers 
	${CMAKE_CURRENT_SOURCE_DIR}/tracy/public/tracy/Tracy.hpp)

list (APPEND projectFiles ${sources} ${headers})

target_definitions()

message(STATUS "Adding target: tracy")
add_library(tracy ${projectFiles})
targetCompileOptions(tracy)

assign_source_group(${projectFiles})
assignIdeFolder(tracy Engine/3rdParty)
libOutputDirectories(tracy)

target_link_libraries (tracy PRIVATE engine_thirdparty_full_policy)

include_directories("${ENGINE_SOURCE_DIR}")
target_include_directories (tracy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
