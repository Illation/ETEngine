

######################
# VCPKG fetches gtkmm
######################


# set up vars
#############

set(_dep "gtkmm")

if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
	set(_p "x64")	
	if(MSVC)
		set(_vcpkgTarget "x64-windows")	
	else()
		set(_vcpkgTarget "x64-linux")	
	endif()
else() # 32 bit
	set(_p "x32")	
	set(_vcpkgTarget "x86-windows")	
endif()

set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
set(_buildDir "../build/${_p}")
set(_vcpkgExe "${_modDir}/vcpkg.exe")
set(_tripletDir "${_modDir}/triplets")

list (APPEND _configs "Debug" "Release")

set(_installDir "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(_outIncDir "${_installDir}/include/gtkmm")
set(_outLibDir "${_installDir}/${_p}/gtkmm")

# make sure vcpkg is built
if(NOT EXISTS "${_vcpkgExe}")
    message(STATUS "Bootstrapping VCPKG")
    message(STATUS "--------------------")
    # on other OS we should use the sh script here
    execute_process(COMMAND ${_modDir}/bootstrap-vcpkg.bat
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _bootVcpkgResult)
    if(NOT _bootVcpkgResult EQUAL "0")
        message(FATAL_ERROR "Failed to bootstreap VCPKG")
    endif()
endif()


if(DEFINED MSVC_TOOLSET_VERSION)

	set(_toolset "v${MSVC_TOOLSET_VERSION}")
	set(_vcpkgTargetToolset "${_vcpkgTarget}-${_toolset}")

	# make sure vcpkg will build for our toolset
	#############################################

	set(_toolsetTriplet "${_tripletDir}/${_vcpkgTargetToolset}.cmake")
	if(NOT EXISTS "${_toolsetTriplet}")

		message(STATUS "creating VCPKG toolset triplet for '${_toolset}'")
		message(STATUS "------------------------------------------------")

		get_filename_component(_compilerRoot ${CMAKE_MAKE_PROGRAM} DIRECTORY)
		get_filename_component(_currentName ${_compilerRoot} NAME)
		while(NOT ${_currentName} MATCHES ".*MSBuild$")
			get_filename_component(_currentName ${_compilerRoot} NAME)
			get_filename_component(_compilerRoot ${_compilerRoot}/.. ABSOLUTE)
		endwhile()

		set(_baseTriplet "${_tripletDir}/${_vcpkgTarget}.cmake")
		file(READ ${_baseTriplet} _baseTripletContent)
		file(WRITE ${_toolsetTriplet} "${_baseTripletContent}")
		file(APPEND ${_toolsetTriplet} "\nset(VCPKG_PLATFORM_TOOLSET ${_toolset})")
		file(APPEND ${_toolsetTriplet} "\nset(VCPKG_VISUAL_STUDIO_PATH \"${_compilerRoot}\")")

	endif()

	# if we don't have the libaries files in the place we expect, build the library
	###############################################################################

	if((NOT EXISTS "${_outIncDir}/") OR (NOT EXISTS "${_outLibDir}/"))

		# build the dependancy through vcpkg
		message(STATUS "Fetching dependancy ${_dep} through VCPKG")
		message(STATUS "-------------------------------------------")
		
		execute_process(COMMAND ${_vcpkgExe} install ${_dep}:${_vcpkgTargetToolset}
						WORKING_DIRECTORY ${_modDir}/
						RESULT_VARIABLE _buildDepResult)
		if(NOT _buildDepResult EQUAL "0")
			message(FATAL_ERROR "Failed to acquire dependancy ${_dep} through VCPKG")
		endif()
		
	endif()

else()
    message(FATAL_ERROR "Failed to run VCPKG because the toolset couldn't be deduced from CMAKE")
endif()
