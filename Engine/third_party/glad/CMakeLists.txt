

#################
# OpenGL Bindings
#################


# set up vars
#############

set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/glad")
set(_buildDir "${CMAKE_CURRENT_SOURCE_DIR}/gl-bindings")
set(_extFile "${CMAKE_CURRENT_SOURCE_DIR}/gl-extensions.txt")

set(_pythonExe "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/vcpkg/downloads/tools/python/python-3.7.3/python.exe") # ensure vcpkg was installed before this file runs
get_filename_component(_pythonExe "${_pythonExe}" ABSOLUTE)

if(NOT EXISTS "${_pythonExe}")
	message(FATAL_ERROR "couldn't find python executable!")
endif()

set(_incDir "${_buildDir}/include")
set(_header "${_incDir}/glad/glad.h")
set(_source "${_buildDir}/src/glad.c")

# if we don't have the libaries files in the place we expect, build the library
###############################################################################

if((NOT EXISTS "${_header}") OR (NOT EXISTS "${_source}"))

	message(STATUS "=================================")
	message(STATUS "Generating GL Bindings with GLAD")
	message(STATUS "=================================")

	# generate project files
	execute_process(COMMAND ${_pythonExe} -m glad --profile=core --out-path=../gl-bindings/ --api=gl=4.5 --generator=c --spec=gl --no-loader --extensions=../gl-extensions.txt
					WORKING_DIRECTORY ${_modDir}/
					RESULT_VARIABLE _genBindings)
	if(NOT _genBindings EQUAL "0")
		message(FATAL_ERROR "Failed to generate GL Bindings - ${_genBindings}")
	endif()

	message(STATUS "=================================")
	message(STATUS "Done generating GL Bindings")
	message(STATUS "=================================")

else()

	message(STATUS "OpenGL bindings already generated!")

endif()

# create a library from the generated files
###########################################

list (APPEND projectFiles ${_header})
list (APPEND projectFiles ${_source})

message(STATUS "Adding target: glad")
add_library(glad ${projectFiles})

assign_source_group(${projectFiles})
set_target_properties(glad PROPERTIES LINKER_LANGUAGE C)

target_include_directories (glad PRIVATE ${_incDir})

libOutputDirectories(glad)
