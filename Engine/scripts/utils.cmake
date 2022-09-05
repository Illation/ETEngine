

##############################
# general for cmake list files
##############################


# custom configuration types
############################
macro(setupConfigurations)

	# set our configuration types
	set(CMAKE_CONFIGURATION_TYPES "Debug;Develop;Shipping" 
		CACHE STRING "Available build-types: Debug, Develop and Shipping" FORCE)
		
	# copy settings from existing build types
	set(CMAKE_CXX_FLAGS_DEVELOP "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
	set(CMAKE_C_FLAGS_DEVELOP "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
	set(CMAKE_EXE_LINKER_FLAGS_DEVELOP "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")

	set(CMAKE_CXX_FLAGS_SHIPPING "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_C_FLAGS_SHIPPING "${CMAKE_C_FLAGS_RELEASE}" )
	set(CMAKE_EXE_LINKER_FLAGS_SHIPPING "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

endmacro(setupConfigurations)


# Platform Architecture
##########################
function(getPlatformArch platform_architecture)

	if (DEFINED CMAKE_VS_PLATFORM_NAME)
		IF("${CMAKE_VS_PLATFORM_NAME}" MATCHES "x64")
			set(${platform_architecture} "x64" PARENT_SCOPE)
		else() # 32 bit
			set(${platform_architecture} "x32" PARENT_SCOPE)
		endif()
	elseif(WIN32)
		if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
			set(${platform_architecture} "x64" PARENT_SCOPE)
		else() # 32 bit
			set(${platform_architecture} "x32" PARENT_SCOPE)
		endif()
	else()
		if(CMAKE_SIZEOF_VOID_P GREATER 4) # 64 bit
			set(${platform_architecture} "x64" PARENT_SCOPE)
		else() # 32 bit
			set(${platform_architecture} "x32" PARENT_SCOPE)
		endif()
	endif()

endfunction(getPlatformArch)


# output dir for executables
############################
function(outputDirectories TARGET _suffix)

	set(_p )
	getPlatformArch(_p)

	foreach(_c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${_c} _C)

		set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${_C} ${PROJECT_DIRECTORY}/bin/${_c}_${_p}/${TARGET}${_suffix})
	endforeach()

endfunction(outputDirectories)


# tool output functions
########################
function(getToolOutputDir TARGET outDir)

	set(_p )
	getPlatformArch(_p)

	set(${outDir} "${PROJECT_DIRECTORY}/bin/tools/${_p}/${TARGET}" PARENT_SCOPE)

endfunction(getToolOutputDir)

########################
function(outputDirectoriesTools TARGET)

	set(_outDir)
	getToolOutputDir(${TARGET} _outDir)

	foreach(_c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${_c} _C)

		set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${_C} ${_outDir})
		set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_NAME_${_C} ${TARGET}_${_c})
	endforeach()

endfunction(outputDirectoriesTools)

########################
function(getToolOutputName TARGET config outName)

	set(_outDir)
	getToolOutputDir(${TARGET} _outDir)

	if(MSVC)
		set(suffix ".exe")
	else()
		set(suffix "")
	endif()
	
	set(${outName} "${_outDir}/${TARGET}_${config}${suffix}" PARENT_SCOPE)

endfunction(getToolOutputName)

########################
function(getToolCopyDir TARGET outDir)

	set(_p )
	getPlatformArch(_p)

	set(${outDir} "${ENGINE_DIRECTORY_ABS}/tools/bin/${_p}/${TARGET}" PARENT_SCOPE)

endfunction(getToolCopyDir)

########################
function(getToolCopyTargetName TARGET outName)

	set(${outName} "try-copy-tool-${TARGET}" PARENT_SCOPE)

endfunction(getToolCopyTargetName)

########################
function(copyToolCommand TARGET dependency)

	set(_toolDir)
	getToolOutputDir(${TARGET} _toolDir)

	set(_copyDir)
	getToolCopyDir(${TARGET} _copyDir)

	set(_target_name)
	getToolCopyTargetName(${TARGET} _target_name)

	# the command list that will run - for installing resources
	#-----------------------------------------------------------
	message(STATUS "Adding target: ${_target_name}")
	add_custom_target(${_target_name} 
		DEPENDS ${dependency} 
		
		COMMAND ${CMAKE_COMMAND} -E echo "copy_directory ${_toolDir} ${_copyDir}"
		COMMAND ${CMAKE_COMMAND} -E copy_directory ${_toolDir} ${_copyDir} || (exit 0) # return true either way in case the tool is running during copy attempt

		VERBATIM
	)
	assignIdeFolder(${_target_name} Engine/Build)

endfunction(copyToolCommand)

########################
function(addToolDependency TARGET TOOL_TARGET)

	set(_copy_target_name)
	getToolCopyTargetName(${TOOL_TARGET} _copy_target_name)

	add_dependencies(${TARGET} ${_copy_target_name})

endfunction(addToolDependency)

########################
function(getToolCookTargetName TARGET outName)

	set(${outName} "cook-tool-data-${TARGET}" PARENT_SCOPE)

endfunction(getToolCookTargetName)

#########################
function(cookToolData TARGET database gen_dir)

	# figure out the directory the cooker binary lives in
	set(_outDir)
	getToolOutputDir(${TARGET} _outDir)

	# compiled source file names
	set(resource_name "compiled_package")
	set(target_files "${gen_dir}${resource_name}.h" "${gen_dir}${resource_name}.cpp")

	set(res_file_engine "${ENGINE_DIRECTORY_ABS}/resources/asset_database_tools.json")

	# any files that can trigger the resources to be rebuilt
	list (APPEND deps ${database})
	list (APPEND deps ${res_file_engine})

	# tool copy target
	set(_copied_tool_cooker)
	getToolCopyTargetName(EtToolCooker _copied_tool_cooker)
	
	set(_tool_cooker_exe)
	getToolOutputName(EtToolCooker $<CONFIG> _tool_cooker_exe)

	# the command lists that will run 
	#---------------------------------

	# first compiled resources

	add_custom_command(
		OUTPUT ${target_files}
		DEPENDS ${deps} ${_copied_tool_cooker}
		COMMAND ${_tool_cooker_exe} -E ${res_file_engine} -P ${database} -O ${gen_dir} -C ${resource_name} --use_pch
		COMMAND ${_tool_cooker_exe} -E ${res_file_engine} -P ${database} -O ${_outDir}/
		COMMENT "Generating tool resource source files and packages: ${res_file_engine} - ${database}; output: ${gen_dir} - ${_outDir}/"
		VERBATIM
	)

	foreach(_target_file ${target_files})
		get_filename_component(_abs_target "${_target_file}" ABSOLUTE)
		set_source_files_properties(${_abs_target} PROPERTIES GENERATED TRUE )
	endforeach()

	set(_tool_cook_target_name)
	getToolCookTargetName(${TARGET} _tool_cook_target_name)

	message(STATUS "Adding target: ${_tool_cook_target_name}")
	add_custom_target(${_tool_cook_target_name} DEPENDS ${target_files} )

	assignIdeFolder(${_tool_cook_target_name} Engine/Build)

	# a command that lets us force cooking the tools resources
	
	set(force_compile_target_name "force-cook-tool-data-compiled-${TARGET}" )
	message(STATUS "Adding target: ${force_compile_target_name}")
	add_custom_target(${force_compile_target_name} 
		DEPENDS ${deps} ${_copied_tool_cooker}
		COMMAND ${_tool_cooker_exe} -E ${res_file_engine} -P ${database} -O ${gen_dir} -C ${resource_name} --use_pch
		COMMENT "Generating tool resource source files: ${res_file_engine} - ${database}; output: ${gen_dir}"
		VERBATIM)
	assignIdeFolder(${force_compile_target_name} Engine/Build)

	set(force_package_target_name "force-cook-tool-data-package-${TARGET}" )
	message(STATUS "Adding target: ${force_package_target_name}")
	add_custom_target(${force_package_target_name} 
		DEPENDS ${deps} ${_copied_tool_cooker}
		COMMAND ${_tool_cooker_exe} -E ${res_file_engine} -P ${database} -O ${_outDir}/
		COMMENT "Generating tool resource source packages: ${res_file_engine} - ${database}; output: ${_outDir}/"
		VERBATIM)
	assignIdeFolder(${force_package_target_name} Engine/Build)

endfunction(cookToolData)


# output dir for libraries
############################
function(libOutputDirectories TARGET)

	set(_p )
	getPlatformArch(_p)

	foreach(_c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${_c} _C)

		set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${_C} ${PROJECT_DIRECTORY}/lib/${_c}_${_p}/${TARGET})
		set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${_C} ${PROJECT_DIRECTORY}/lib/${_c}_${_p}/${TARGET})
	endforeach()

endfunction(libOutputDirectories)


# make project filters mimic directory structure
################################################
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

# place a target inside of an IDE filter
#########################################
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# can't place those targets in custom folder, so increasing visibility by placing them  in the top level
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "") 

macro(assignIdeFolder _target _folder)
	if(MSVC)
		set_property (TARGET "${_target}" PROPERTY FOLDER "${_folder}")
	endif()
endmacro(assignIdeFolder)


# create a general target for the project
###########################################
function(createProjectGeneral)
    
	file(GLOB_RECURSE projectFiles ${PROJECT_DIRECTORY}/resources/*)

	message(STATUS "Adding target: GeneralProject")
	add_custom_target(GeneralProject ALL SOURCES ${projectFiles})
	assign_source_group(${projectFiles})
	assignIdeFolder(GeneralProject Project/General)

endfunction(createProjectGeneral)


# PCH
#########
function(precompiled_headers SOURCELIST)
	# only windows
	if (MSVC)
		# run this first to set all files to use pch
		foreach( src_file ${SOURCELIST} )
			set_source_files_properties( ${src_file} PROPERTIES COMPILE_FLAGS "/Yustdafx.h" )
		endforeach( src_file ${SOURCELIST} )
		# run this second to overwrite the pch setting for the stdafx.cpp file
		set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.h" )
	endif(MSVC)
endfunction(precompiled_headers)


# Parallel build
#################
function(targetCompileOptions _target)
	# only windows
	if(MSVC)
		target_compile_options(${_target} PRIVATE "/MP" "/DWIN32_LEAN_AND_MEAN")
	endif()

	if(MSVC)
		target_compile_options(
			${_target} PRIVATE
			"$<$<CONFIG:Debug>:/D_DEBUG>"
			"$<$<CONFIG:Debug>:/DET_DEBUG>"
			"$<$<CONFIG:Develop>:/DET_DEVELOP>"
			"$<$<CONFIG:Shipping>:/DET_SHIPPING>"
			"/DRMLUI_STATIC_LIB"
		)
	else()
		target_compile_options(
			${_target} PRIVATE
			"$<$<CONFIG:Debug>:-D_DEBUG>"
			"$<$<CONFIG:Debug>:-DET_DEBUG>"
			"$<$<CONFIG:Develop>:-DET_DEVELOP>"
			"$<$<CONFIG:Shipping>:-DET_SHIPPING>"
			"-DRMLUI_STATIC_LIB"
		)
	endif()

	target_compile_definitions(${_target} PRIVATE "-DIMGUI_USER_CONFIG=<imconfig.h>")
endfunction(targetCompileOptions)


# Debug visualizers
####################
function(addDebugVisualizers _target)
	if (MSVC)
		target_sources(${_target} PUBLIC ${ENGINE_DIRECTORY_ABS}/tools/et-visualizers.natvis)
	endif()
endfunction()


# Config defines
#################
function(target_definitions)
	# os
	if (MSVC)
		add_definitions(-DET_PLATFORM_WIN)
	else()
		add_definitions(-DET_PLATFORM_LINUX)
	endif(MSVC)
	
	set(_p )
	getPlatformArch(_p)
	
	# architecture
	if("${_p}" MATCHES "x64") # 64 bit
		add_definitions(-DET_ARCH_X64)
	 else() 
		add_definitions(-DET_ARCH_X32)
	endif()
endfunction(target_definitions)


# get vcpkg install directory
####################################################
function(getVcpkgTarget vcpkg_target)

	set(_p )
	getPlatformArch(_p)

	if("${_p}" MATCHES "x64") # 64 bit
		if(MSVC)
			set(${vcpkg_target} "x64-windows" PARENT_SCOPE)
		else()
			set(${vcpkg_target} "x64-linux" PARENT_SCOPE)
		endif()
	else() # 32 bit
		set(${vcpkg_target} "x86-windows" PARENT_SCOPE)
	endif()

endfunction(getVcpkgTarget)


# get vcpkg toolset triplet
####################################################
function(getToolsetTriplet out_triplet)

	set(_vcpkgTarget )
	getVcpkgTarget(_vcpkgTarget)
	
	if(ETE_SINGLE_CONFIG)
		string(TOLOWER ${ETE_BUILD_LIB_CONFIG} _config)
		# set(_config "-${_config}")
		set(_config "")
	else()
		set(_config "")
	endif()

	if(DEFINED MSVC_TOOLSET_VERSION)
		set(_toolset "v${MSVC_TOOLSET_VERSION}")
	elseif(DEFINED CMAKE_VS_PLATFORM_TOOLSET)
		set(_toolset "${CMAKE_VS_PLATFORM_TOOLSET}")
	elseif(UNIX)
		set(_toolset "")
	else()
		message(FATAL_ERROR "Visual studio toolset couldn't be deduced from cmake")
	endif()

	set(_out_triplet "${_vcpkgTarget}")
	if(NOT "${_toolset}" STREQUAL "")
		set(_out_triplet "${_out_triplet}-${_toolset}")
	endif()
	set(_out_triplet "${_out_triplet}${_config}")

	set(${out_triplet} "${_out_triplet}" PARENT_SCOPE)
endfunction(getToolsetTriplet)


# get vcpkg install directory
####################################################
function(getVcpkgInstallDir vcpkg_install)
	set(_vcpkgTargetToolset )
	getToolsetTriplet(_vcpkgTargetToolset)

	set(${vcpkg_install} "${ENGINE_DIRECTORY_ABS}/third_party/vcpkg/vcpkg/installed/${_vcpkgTargetToolset}" PARENT_SCOPE)
endfunction(getVcpkgInstallDir)


# bullet output directory
##########################
function(getBulletBuildDir bullet_build)

	set(_p )
	getPlatformArch(_p)

	set(${bullet_build} "${ENGINE_DIRECTORY_ABS}/third_party/bullet/build/${_p}" PARENT_SCOPE)
endfunction(getBulletBuildDir)


# rmlui output directory
##########################
function(getRmlUiBuildDir rmlui_build)

	set(_p )
	getPlatformArch(_p)

	set(${rmlui_build} "${ENGINE_DIRECTORY_ABS}/third_party/rmlui/build/${_p}" PARENT_SCOPE)
endfunction(getRmlUiBuildDir)


# rttr output directory
##########################
function(getRttrBuildDir rttr_build)

	set(_p )
	getPlatformArch(_p)

	set(${rttr_build} "${ENGINE_DIRECTORY_ABS}/third_party/rttr/build/${_p}" PARENT_SCOPE)
endfunction(getRttrBuildDir)


# openal output directory
##########################
function(getOpenAlBuildDir openal_build)

	set(_p )
	getPlatformArch(_p)

	set(${openal_build} "${ENGINE_DIRECTORY_ABS}/third_party/openal/build/${_p}" PARENT_SCOPE)
endfunction(getOpenAlBuildDir)


# glfw output directory
##########################
function(getGlfwBuildDir glfw_build)

	set(_p )
	getPlatformArch(_p)

	set(${glfw_build} "${ENGINE_DIRECTORY_ABS}/third_party/glfw/build/${_p}" PARENT_SCOPE)
endfunction(getGlfwBuildDir)


# link to all runtime dependencies
###################################
function(dependancyLinks TARGET)

	set(_glfwBuild )
	getGlfwBuildDir(_glfwBuild)
	
	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)
	
	set(_bulletBuild )
	getBulletBuildDir(_bulletBuild)
	
	set(_rttrBuild )
	getRttrBuildDir(_rttrBuild)

	set(_alBuild )
	getOpenAlBuildDir(_alBuild)

	set(_rmluiBuild )
	getRmlUiBuildDir(_rmluiBuild)

	# separate debug and release libs
	target_link_libraries (${TARGET} 		
		debug ${_glfwBuild}/src/Debug/glfw3.lib						optimized ${_glfwBuild}/src/Debug/glfw3.lib

		debug ${_rttrBuild}/install/lib/librttr_core_d.lib			optimized ${_rttrBuild}/install/lib/librttr_core.lib
	
		debug ${_bulletBuild}/lib/Debug/BulletDynamics_Debug.lib	optimized ${_bulletBuild}/lib/Release/BulletDynamics.lib
		debug ${_bulletBuild}/lib/Debug/BulletCollision_Debug.lib	optimized ${_bulletBuild}/lib/Release/BulletCollision.lib
		debug ${_bulletBuild}/lib/Debug/LinearMath_Debug.lib		optimized ${_bulletBuild}/lib/Release/LinearMath.lib 

		debug ${_alBuild}/Debug/OpenAL32.lib						optimized ${_alBuild}/Release/OpenAL32.lib

		debug ${_rmluiBuild}/Debug/RmlCore.lib						optimized ${_rmluiBuild}/Release/RmlCore.lib
		debug ${_rmluiBuild}/Debug/RmlDebugger.lib

		debug ${_vcpkgInstall}/debug/lib/zlibd.lib					optimized ${_vcpkgInstall}/lib/zlib.lib)

	if (MSVC)
		target_link_libraries(${TARGET} opengl32.lib)
	endif(MSVC)

endfunction(dependancyLinks)

# link to all cooker dependencies
###################################
function(cookerLinks TARGET)
	
	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)

	target_link_libraries (${TARGET} 		
		debug ${_vcpkgInstall}/debug/lib/freetyped.lib				optimized ${_vcpkgInstall}/lib/freetype.lib)

endfunction(cookerLinks)


# link to all editor dependencies
###################################
function(editorLinks TARGET)

	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)
	
	set(_dbg "${_vcpkgInstall}/debug/lib/")
	set(_rel "${_vcpkgInstall}/lib/")

	target_link_libraries (${TARGET} 		
		debug ${_dbg}bz2d.lib				optimized ${_rel}bz2.lib			
		debug ${_dbg}cairod.lib				optimized ${_rel}cairo.lib	
		debug ${_dbg}cairo-gobjectd.lib		optimized ${_rel}cairo-gobject.lib	
		debug ${_dbg}libpng16d.lib			optimized ${_rel}libpng16.lib	
		debug ${_dbg}pcre16d.lib			optimized ${_rel}pcre16.lib	
		debug ${_dbg}pcre32d.lib			optimized ${_rel}pcre32.lib	
		debug ${_dbg}pcrecppd.lib			optimized ${_rel}pcrecpp.lib	
		debug ${_dbg}pcred.lib				optimized ${_rel}pcre.lib	
		debug ${_dbg}pcreposixd.lib			optimized ${_rel}pcreposix.lib	
		debug ${_dbg}pixman-1d.lib			optimized ${_rel}pixman-1.lib	

		debug ${_dbg}atk-1.0.lib			optimized ${_rel}atk-1.0.lib	 
		debug ${_dbg}atkmm-1.6.lib			optimized ${_rel}atkmm-1.6.lib	 
		debug ${_dbg}cairomm-1.0.lib		optimized ${_rel}cairomm-1.0.lib	 
		debug ${_dbg}epoxy.lib				optimized ${_rel}epoxy.lib	 
		debug ${_dbg}expat.lib				optimized ${_rel}expat.lib	 
		debug ${_dbg}fontconfig.lib			optimized ${_rel}fontconfig.lib	 
		debug ${_dbg}gailutil-3.0.lib		optimized ${_rel}gailutil-3.0.lib	 
		debug ${_dbg}gdk_pixbuf-2.0.lib		optimized ${_rel}gdk_pixbuf-2.0.lib	 
		debug ${_dbg}gdk-3.0.lib			optimized ${_rel}gdk-3.0.lib	 
		debug ${_dbg}gdkmm-3.0.lib			optimized ${_rel}gdkmm-3.0.lib	 
		debug ${_dbg}gio-2.0.lib			optimized ${_rel}gio-2.0.lib	 
		debug ${_dbg}giomm.lib				optimized ${_rel}giomm.lib	 
		debug ${_dbg}glib-2.0.lib			optimized ${_rel}glib-2.0.lib	 
		debug ${_dbg}glibmm.lib				optimized ${_rel}glibmm.lib	 
		debug ${_dbg}gmodule-2.0.lib		optimized ${_rel}gmodule-2.0.lib	 
		debug ${_dbg}gobject-2.0.lib		optimized ${_rel}gobject-2.0.lib	 
		debug ${_dbg}gthread-2.0.lib		optimized ${_rel}gthread-2.0.lib	 
		debug ${_dbg}gtk-3.0.lib			optimized ${_rel}gtk-3.0.lib	 
		debug ${_dbg}gtkmm-3.0.lib			optimized ${_rel}gtkmm-3.0.lib	 
		debug ${_dbg}harfbuzz.lib			optimized ${_rel}harfbuzz.lib	 
		debug ${_dbg}libcharset.lib			optimized ${_rel}libcharset.lib	 
		debug ${_dbg}libffi.lib				optimized ${_rel}libffi.lib	 
		debug ${_dbg}libiconv.lib			optimized ${_rel}libiconv.lib	 
		debug ${_dbg}libintl.lib			optimized ${_rel}libintl.lib	 
		debug ${_dbg}pango-1.0.lib			optimized ${_rel}pango-1.0.lib	 
		debug ${_dbg}pangocairo-1.0.lib		optimized ${_rel}pangocairo-1.0.lib	 
		debug ${_dbg}pangoft2-1.0.lib		optimized ${_rel}pangoft2-1.0.lib	 
		debug ${_dbg}pangomm.lib			optimized ${_rel}pangomm.lib	 
		debug ${_dbg}pangowin32-1.0.lib		optimized ${_rel}pangowin32-1.0.lib	 
		debug ${_dbg}sigc-2.0.lib			optimized ${_rel}sigc-2.0.lib	 )

endfunction(editorLinks)


# place a list of all libraries built by vcpkg
####################################################
function(getVcpkgLibs out_list target_type)
	set(_libs )
	if (NOT "${target_type}" STREQUAL "no_runtime")
		list (APPEND _libs "bz2" "libpng16" "zlib" "zlibd")
	endif()
	if (NOT "${target_type}" STREQUAL "runtime")
		list (APPEND _libs "freetype")
	endif()
	set (${out_list} ${_libs} PARENT_SCOPE)
endfunction(getVcpkgLibs)


# third party includes
###########################
function(libIncludeDirs)

	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)
	include_directories("${_vcpkgInstall}/include/")	
	
	set(_rttrBuild )
	getRttrBuildDir(_rttrBuild)
	include_directories("${_rttrBuild}/install/include/")	
		
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/stb")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/mikkt")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/glad/gl-bindings/include")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/bullet/bullet3/src")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/openal/openal-soft/include")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/glfw/glfw/include")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/rmlui/RmlUi/Include")

endfunction(libIncludeDirs)


# check if filenames loosely match
###################################
function(getMatchingFiles _searchList _fileList out_list)

	foreach(_searchFile ${_searchList})

		set(_index -1)
		set(_currentIndex -1)
		foreach(_filePath ${_fileList})
			MATH(EXPR _currentIndex "${_currentIndex}+1")
			get_filename_component(_fileName ${_filePath} NAME_WE)
			if ("${_searchFile}" MATCHES "${_fileName}*")
				set(_index ${_currentIndex})
			endif()
		endforeach()

		if(${_index} GREATER -1)
			list(GET _fileList ${_index} _filePath)
			list(APPEND ret_list ${_filePath})
		endif()

	endforeach()
	
	set (${out_list} "${ret_list}" PARENT_SCOPE)

endfunction(getMatchingFiles)


# copy dll (and pdb) files in the appropriate directory according to configuration - post build command version
################################################################################################################
function(copyDllCommand _target)
	
	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)
	set(vcpkg_libs )
	getVcpkgLibs(vcpkg_libs default)

	set(_alBuild )
	getOpenAlBuildDir(_alBuild)

	set(_rmluiBuild )
	getRmlUiBuildDir(_rmluiBuild)

	# where the lib files live
	set(_cfg "Release") 
	set(_vcCfg "")
	if("$<CONFIG>" STREQUAL "Debug")
		set(_cfg "Debug")
		set(_vcCfg "/debug")
	
		# for debug applications we also copy pdbs	
		file(GLOB pdbs ${_vcpkgInstall}${_vcCfg}/bin/*.pdb)
		getMatchingFiles("${vcpkg_libs}" "${pdbs}" pdbs)
		foreach(_pdb ${pdbs})
			add_custom_command(TARGET ${_target} 
				POST_BUILD
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_pdb}" $<TARGET_FILE_DIR:${_target}>
				COMMAND ${CMAKE_COMMAND} -E echo "Copying ${_pdb}" 
			)
		endforeach()

		add_custom_command(TARGET ${_target} 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_alBuild}/Debug/OpenAL32.pdb" $<TARGET_FILE_DIR:${_target}>
			COMMAND ${CMAKE_COMMAND} -E echo "Copying ${_alBuild}/Debug/OpenAL32.pdb" 
		)

		add_custom_command(TARGET ${_target} 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_rmluiBuild}/Debug/RmlCore.pdb" $<TARGET_FILE_DIR:${_target}>
			COMMAND ${CMAKE_COMMAND} -E echo "Copying ${_rmluiBuild}/Debug/RmlCore.pdb" 
		)

		add_custom_command(TARGET ${_target} 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${_rmluiBuild}/Debug/RmlDebugger.pdb" $<TARGET_FILE_DIR:${_target}>
			COMMAND ${CMAKE_COMMAND} -E echo "Copying ${_rmluiBuild}/Debug/RmlDebugger.pdb" 
		)
	endif()
	
	file(GLOB debugDlls ${_vcpkgInstall}/debug/bin/*.dll)
	getMatchingFiles("${vcpkg_libs}" "${debugDlls}" debugDlls)
	foreach(_dll ${debugDlls})
		add_custom_command(TARGET ${_target} 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,copy_if_different\ "${_dll}"\ $<TARGET_FILE_DIR:${_target}>,echo\ "">
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,echo\ "Copying ${_dll}",echo\ "">
		)
	endforeach()
	file(GLOB releaseDlls ${_vcpkgInstall}/bin/*.dll)
	getMatchingFiles("${vcpkg_libs}" "${releaseDlls}" releaseDlls)
	foreach(_dll ${releaseDlls})
		add_custom_command(TARGET ${_target} 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,echo\ "",copy_if_different\ "${_dll}"\ $<TARGET_FILE_DIR:${_target}>>
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,echo\ "",echo\ "Copying ${_dll}">
		)
	endforeach()
	
	add_custom_command(TARGET ${_target} 
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,copy_if_different\ "${_alBuild}/Debug/OpenAL32.dll"\ $<TARGET_FILE_DIR:${_target}>,echo\ "">
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,echo\ "Copying ${_alBuild}/Debug/OpenAL32.dll",echo\ "">
	)
	add_custom_command(TARGET ${_target} 
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,echo\ "",copy_if_different\ "${_alBuild}/Release/OpenAL32.dll"\ $<TARGET_FILE_DIR:${_target}>>
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<CONFIG:Debug>,echo\ "",echo\ "Copying ${_alBuild}/Release/OpenAL32.dll">
	)

endfunction(copyDllCommand)


# install dll (and pdb) files in the appropriate directory according to configuration
######################################################################################
function(installDlls TARGET _suffix)

	set(baseBinDir "${PROJECT_DIRECTORY}/bin")

	# paths for our libraries depend on the architecture we compile for
	set(_p )
	getPlatformArch(_p)
	
	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)
	set(vcpkg_libs )
	getVcpkgLibs(vcpkg_libs runtime)

	set(_alBuild )
	getOpenAlBuildDir(_alBuild)

	set(_rmluiBuild )
	getRmlUiBuildDir(_rmluiBuild)

	foreach(configType ${CMAKE_CONFIGURATION_TYPES})

		set(binDir "${baseBinDir}/${configType}_${_p}/${TARGET}${_suffix}")

		# where the lib files live
		set(libcfg "Release") 
		set(_vcCfg "")
		if("${configType}" STREQUAL "Debug")
			set(libcfg "Debug")
			set(_vcCfg "/debug")

			# for debug applications we also copy pdbs
			file(GLOB pdbs ${_vcpkgInstall}${_vcCfg}/bin/*.pdb)
			getMatchingFiles("${vcpkg_libs}" "${pdbs}" pdbs)
			foreach(_pdb ${pdbs})
				install(FILES ${_pdb} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
			endforeach()
			
			install(FILES ${_alBuild}/Debug/OpenAL32.pdb CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
			
			install(FILES ${_rmluiBuild}/Debug/RmlCore.pdb CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
			install(FILES ${_rmluiBuild}/Debug/RmlDebugger.pdb CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
		endif()

		# copy dlls for all libraries		
		file(GLOB dlls ${_vcpkgInstall}${_vcCfg}/bin/*.dll)
		getMatchingFiles("${vcpkg_libs}" "${dlls}" dlls)
		foreach(_dll ${dlls})
			install(FILES ${_dll} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
		endforeach()
		
		install(FILES ${_alBuild}/${libcfg}/OpenAL32.dll CONFIGURATIONS ${configType} DESTINATION ${binDir}/)

	endforeach()

endfunction(installDlls)


# install dll (and pdb) files in the appropriate directory according to configuration
######################################################################################
function(installEditorDlls TARGET)

	set(baseBinDir "${PROJECT_DIRECTORY}/bin")

	# paths for our libraries depend on the architecture we compile for
	set(_p )
	getPlatformArch(_p)
	
	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)
	set(vcpkg_libs )
	getVcpkgLibs(vcpkg_libs no_runtime)

	foreach(configType ${CMAKE_CONFIGURATION_TYPES})

		set(binDir "${baseBinDir}/${configType}_${_p}/${TARGET}/bin")

		set(_vcCfg "")
		if("${configType}" STREQUAL "Debug")
			set(_vcCfg "/debug")
		endif()

		if(("${configType}" STREQUAL "Debug") OR ("${configType}" STREQUAL "Develop"))
			# for debug applications we also copy pdbs
			file(GLOB pdbs ${_vcpkgInstall}${_vcCfg}/bin/*.pdb)
			foreach(_pdb ${pdbs})
				install(FILES ${_pdb} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
			endforeach()
			
			# for debug applications we also copy pdbs
			file(GLOB more_pdbs ${_vcpkgInstall}${_vcCfg}/bin/*.pdb)
			getMatchingFiles("${vcpkg_libs}" "${pdbs}" more_pdbs)
			foreach(_pdb ${more_pdbs})
				install(FILES ${_pdb} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
			endforeach()
		endif()

		# copy dlls for all libraries		
		file(GLOB dlls ${_vcpkgInstall}${_vcCfg}/bin/*.dll)
		foreach(_dll ${dlls})
			install(FILES ${_dll} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
		endforeach()

		# copy dlls for all libraries		
		file(GLOB more_dlls ${_vcpkgInstall}${_vcCfg}/bin/*.dll)
		getMatchingFiles("${vcpkg_libs}" "${dlls}" more_dlls)
		foreach(_dll ${more_dlls})
			install(FILES ${_dll} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
		endforeach()

	endforeach()

endfunction(installEditorDlls)


# sets up the user directory and config data
#############################################
function(installConfig TARGET)

	set(baseBinDir "${PROJECT_DIRECTORY}/bin")
	set(tempBuildDir "${PROJECT_DIRECTORY}/build/temp")
	set(configDir "${PROJECT_DIRECTORY}/resources/config")

	# paths for our libraries depend on the architecture we compile for
	set(_p )
	getPlatformArch(_p)

	# user directory where user data and configuration
	set(userDir "${PROJECT_DIRECTORY}/user_data")

	# create a file within the packaged resources that points to the user directory
	get_filename_component(absUserDir ${userDir} ABSOLUTE)
	file(WRITE ${tempBuildDir}/config/userDirPointer.json "{\"dir pointer\":{\"user dir path\":\"${absUserDir}/\"}}" )
		
	# copy config files
	install(DIRECTORY ${configDir}/ DESTINATION ${userDir}/)

	# copy user dir pointer to output directory
	foreach(configType ${CMAKE_CONFIGURATION_TYPES})
		install(FILES ${tempBuildDir}/config/userDirPointer.json 
			CONFIGURATIONS ${configType} 
			DESTINATION ${baseBinDir}/${configType}_${_p}/${TARGET}/)
	endforeach()

endfunction(installConfig)


# install everything in the appropriate directory according to configuration
#############################################################################
function(installEditorResources TARGET)

	set(baseBinDir "${PROJECT_DIRECTORY}/bin")
	set(tempBuildDir "${PROJECT_DIRECTORY}/build/temp")

	# paths for our libraries depend on the architecture we compile for
	set(_p )
	getPlatformArch(_p)

	set(_edPackagedDir "${ENGINE_DIRECTORY_ABS}/third_party/gtk/GTK-for-Windows-Runtime-Environment-Installer/gtk-nsis-pack")
	
	file(WRITE ${tempBuildDir}/config/dirPointers.json 
		"{\"dir pointers\":{\"project dir path\":\"${PROJECT_DIRECTORY}/\",\"engine dir path\":\"${ENGINE_DIRECTORY_ABS}/\"}}" )
		
	set(userDirEditor "${PROJECT_DIRECTORY}/user_data/editor")
		
	# copy config files
	install(DIRECTORY ${ENGINE_DIRECTORY_ABS}/config/editor/layouts/ DESTINATION ${userDirEditor}/layouts/)

	foreach(configType ${CMAKE_CONFIGURATION_TYPES})

		set(binDir "${baseBinDir}/${configType}_${_p}/${TARGET}")

		# config files
		install(FILES ${tempBuildDir}/config/dirPointers.json 
			CONFIGURATIONS ${configType} 
			DESTINATION ${binDir}/config/)

		# pixbuf loaders
		install(DIRECTORY ${_edPackagedDir}/lib/gdk-pixbuf-2.0/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/lib/gdk-pixbuf-2.0/)

		# gtk config
		install(DIRECTORY ${ENGINE_DIRECTORY_ABS}/config/gtk-3.0/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/share/gtk-3.0/)
			
		# generated share
		install(DIRECTORY ${PROJECT_DIRECTORY}/build/temp/share/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/share/)
			
		# icons
		install(DIRECTORY ${_edPackagedDir}/share/icons/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/share/icons/)
			
	endforeach()

endfunction(installEditorResources)


# cook package files and install them in the binary output directory
#####################################################################
function(installCookResources TARGET)

	set(cmp_dir "${PROJECT_DIRECTORY}/resources/")
	set(cmp_dir_engine "${ENGINE_DIRECTORY_ABS}/resources/")

	# figure out the directory the cooker binary lives in
	#-----------------------------------------------------------
	set(_p )
	getPlatformArch(_p)
	set(bin_base_dir "${PROJECT_DIRECTORY}/bin/$<CONFIG>_${_p}/")
	set(cooker_dir "${bin_base_dir}ProjectCooker/")
	set(pak_file_dir "${bin_base_dir}${TARGET}/")

	set(resource_name "compiledPackage")
	set(res_file "${cmp_dir}asset_database.json")
	set(res_file_engine "${cmp_dir_engine}asset_database.json")

	# any files that can trigger the resources to be rebuilt
	file(GLOB_RECURSE deps ${cmp_dir}/assets/*.* ${cmp_dir_engine}/assets/*.*)
	list (APPEND deps ${res_file})
	list (APPEND deps ${res_file_engine})

	set(target_name "cook-installed-resources-${TARGET}")

	# the command list that will run - for installing resources
	#-----------------------------------------------------------
	message(STATUS "Adding target: ${target_name}")
	add_custom_target(${target_name} 
		DEPENDS ${deps} ProjectCooker 
		
		COMMAND ${CMAKE_COMMAND} -E echo "Cooking resource packages - Source ${res_file} ; Out directory: ${pak_file_dir}"
		COMMAND ${CMAKE_COMMAND} -E echo ""
		COMMAND ${CMAKE_COMMAND} -E echo "${cooker_dir}ProjectCooker.exe -E ${res_file_engine} -P ${res_file} -O ${pak_file_dir}"
		COMMAND ${cooker_dir}ProjectCooker.exe -E ${res_file_engine} -P ${res_file} -O ${pak_file_dir}
		COMMAND ${CMAKE_COMMAND} -E echo ""
		COMMAND ${CMAKE_COMMAND} -E echo ""
		
		COMMENT "Cooking installed resource files"

		VERBATIM
	)
	assignIdeFolder(${target_name} Project/ContentPipeline)

endfunction(installCookResources)
