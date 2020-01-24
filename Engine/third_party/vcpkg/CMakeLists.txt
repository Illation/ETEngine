

######################
# VCPKG fetches gtkmm
######################


# set up vars
#############

set(_dep "gtkmm")
# this will also include freetype

set(_vcpkgTarget )
getVcpkgTarget(_vcpkgTarget)

set(_modDir "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg")
set(_buildDir "../build/${_p}")
if(WIN32)
    set(_vcpkgExe "${_modDir}/vcpkg.exe")
    set(_vcpkgBootstrap "${_modDir}/bootstrap-vcpkg.bat")
else()
    set(_vcpkgExe "${_modDir}/vcpkg")
    set(_vcpkgBootstrap "${_modDir}/bootstrap-vcpkg.sh")
endif()
set(_tripletDir "${_modDir}/triplets")

# ensure we have a vcpkg exe
#############################
if(NOT EXISTS "${_vcpkgExe}")
    message(STATUS "Bootstrapping VCPKG")
    message(STATUS "--------------------")
    # on other OS we should use the sh script here
    execute_process(COMMAND ${_vcpkgBootstrap}
                    WORKING_DIRECTORY ${_modDir}/
                    RESULT_VARIABLE _bootVcpkgResult)
    if(NOT _bootVcpkgResult EQUAL "0")
        message(FATAL_ERROR "Failed to bootstrap VCPKG")
    endif()
else()

	message(STATUS "Library executable already build: VCPKG")

endif()


# deduce the toolset
####################
set(_vcpkgTargetToolset )
getToolsetTriplet(_vcpkgTargetToolset)


# make sure vcpkg will build for our toolset
#############################################

set(_toolsetTriplet "${_tripletDir}/${_vcpkgTargetToolset}.cmake")
if(NOT EXISTS "${_toolsetTriplet}")

	if(DEFINED MSVC_TOOLSET_VERSION)
		set(_toolset "v${MSVC_TOOLSET_VERSION}")
	elseif(DEFINED CMAKE_VS_PLATFORM_TOOLSET)
		set(_toolset "${CMAKE_VS_PLATFORM_TOOLSET}")
	else()
		message(FATAL_ERROR "Visual studio toolset couldn't be deduced from cmake")
	endif()

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

#	if(ETE_SINGLE_CONFIG)
#		string(TOLOWER ${ETE_BUILD_LIB_CONFIG} _config)
#		file(APPEND ${_toolsetTriplet} "\nset(VCPKG_BUILD_TYPE ${_config})")
#	endif()

else()

	message(STATUS "using existing VCPKG triplet: ${_toolsetTriplet}")

endif()

# if we don't have the libaries files in the place we expect, build the library
###############################################################################

# build the dependency through vcpkg - if it is already built vcpkg will skip
message(STATUS "Fetching dependency ${_dep} through VCPKG")
message(STATUS "-------------------------------------------")

execute_process(COMMAND ${_vcpkgExe} install ${_dep}:${_vcpkgTargetToolset}
				WORKING_DIRECTORY ${_modDir}/
				RESULT_VARIABLE _buildDepResult)
if(NOT _buildDepResult EQUAL "0")
	message(FATAL_ERROR "Failed to acquire dependency ${_dep} through VCPKG")
endif()
