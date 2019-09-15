

##############################
# general for cmake list files
##############################


# custom configuration types
############################
macro(setupConfigurations)

	# set our configuration types
	set(CMAKE_CONFIGURATION_TYPES "Debug;DebugEditor;Develop;DevelopEditor;Shipping" 
		CACHE STRING "Available build-types: Debug, DebugEditor, Develop, DevelopEditor and Shipping" FORCE)
		
	# copy settings from existing build types
	set(CMAKE_CXX_FLAGS_DEBUGEDITOR "${CMAKE_CXX_FLAGS_DEBUG}" )
	set(CMAKE_C_FLAGS_DEBUGEDITOR "${CMAKE_C_FLAGS_DEBUG}")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUGEDITOR "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

	set(CMAKE_CXX_FLAGS_DEVELOP "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
	set(CMAKE_C_FLAGS_DEVELOP "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
	set(CMAKE_EXE_LINKER_FLAGS_DEVELOP "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")

	set(CMAKE_CXX_FLAGS_DEVELOPEDITOR "${CMAKE_CXX_FLAGS_DEVELOP}")
	set(CMAKE_C_FLAGS_DEVELOPEDITOR "${CMAKE_C_FLAGS_DEVELOP}")
	set(CMAKE_EXE_LINKER_FLAGS_DEVELOPEDITOR "${CMAKE_EXE_LINKER_FLAGS_DEVELOP}")

	set(CMAKE_CXX_FLAGS_SHIPPING "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_C_FLAGS_SHIPPING "${CMAKE_C_FLAGS_RELEASE}" )
	set(CMAKE_EXE_LINKER_FLAGS_SHIPPING "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

endmacro(setupConfigurations)


# output dir for executables
############################
function(outputDirectories TARGET _suffix)

	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8") # 64 bit
		set(_p "x64")	
	else() # 32 bit
		set(_p "x32")	
	endif()

	foreach(_c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${_c} _C)

		set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${_C} ${PROJECT_DIRECTORY}/bin/${_c}_${_p}/${TARGET}${_suffix})
	endforeach()

endfunction(outputDirectories)


# output dir for libraries
############################
function(libOutputDirectories TARGET)

	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8") # 64 bit
		set(_p "x64")	
	else() # 32 bit
		set(_p "x32")	
	endif()

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
		target_compile_options(${_target} PRIVATE "/MP")
	endif()

	target_compile_options(
		${_target} PRIVATE 
		"$<$<CONFIG:Debug>:/D_DEBUG /DET_DEBUG>"
		"$<$<CONFIG:DebugEditor>:/D_DEBUG /DET_DEBUG /DEDITOR>"
		"$<$<CONFIG:Develop>:/DET_DEVELOP>"
		"$<$<CONFIG:DevelopEditor>:/DET_DEVELOP /DEDITOR>"
		"$<$<CONFIG:Shipping>:/DET_SHIPPING>"
	)
endfunction(targetCompileOptions)


# Config defines
#################
function(target_definitions)
	# os
	if (MSVC)
		add_definitions(-DPLATFORM_Win)
	endif(MSVC)
	
	# architecture
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		add_definitions(-DPLATFORM_x64)
	 else() 
		add_definitions(-DPLATFORM_x32)
	endif()
endfunction(target_definitions)


# get vcpkg install directory
####################################################
function(getVcpkgTarget vcpkg_target)

	if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
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
		set(_config "-${_config}")
	else()
		set(_config "")
	endif()

	if(DEFINED MSVC_TOOLSET_VERSION)
		set(_toolset "v${MSVC_TOOLSET_VERSION}")
	elseif(DEFINED CMAKE_VS_PLATFORM_TOOLSET)
		set(_toolset "${CMAKE_VS_PLATFORM_TOOLSET}")
	else()
		message(FATAL_ERROR "Visual studio toolset couldn't be deduced from cmake")
	endif()

	set(${out_triplet} "${_vcpkgTarget}-${_toolset}${_config}" PARENT_SCOPE)
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
	if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
		set(_p "x64")
	else() # 32 bit
		set(_p "x32")
	endif()

	set(${bullet_build} "${ENGINE_DIRECTORY_ABS}/third_party/bullet/build/${_p}" PARENT_SCOPE)
endfunction(getBulletBuildDir)


# rttr output directory
##########################
function(getRttrBuildDir rttr_build)
	if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
		set(_p "x64")
	else() # 32 bit
		set(_p "x32")
	endif()

	set(${rttr_build} "${ENGINE_DIRECTORY_ABS}/third_party/rttr/build/${_p}" PARENT_SCOPE)
endfunction(getRttrBuildDir)


# rttr output directory
##########################
function(getAssimpBuildDir assimp_build)
	if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
		set(_p "x64")
	else() # 32 bit
		set(_p "x32")
	endif()

	set(${assimp_build} "${ENGINE_DIRECTORY_ABS}/third_party/assimp/build/${_p}" PARENT_SCOPE)
endfunction(getAssimpBuildDir)


# rttr output directory
##########################
function(getOpenAlBuildDir openal_build)
	if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
		set(_p "x64")
	else() # 32 bit
		set(_p "x32")
	endif()

	set(${openal_build} "${ENGINE_DIRECTORY_ABS}/third_party/openal/build/${_p}" PARENT_SCOPE)
endfunction(getOpenAlBuildDir)


# rttr output directory
##########################
function(getGlfwBuildDir glfw_build)
	if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)") # 64 bit
		set(_p "x64")
	else() # 32 bit
		set(_p "x32")
	endif()

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

	set(_assimpBuild )
	getAssimpBuildDir(_assimpBuild)
	file(GLOB _assimpDebugLib ${_assimpBuild}/code/Debug/*.lib)		# assimp defines the toolset in the lib name, so we just glob it
	file(GLOB _assimpReleaseLib ${_assimpBuild}/code/Release/*.lib)

	set(_alBuild )
	getOpenAlBuildDir(_alBuild)

	# separate debug and release libs
	target_link_libraries (${TARGET} 		
		debug ${_glfwBuild}/src/Debug/glfw3.lib						optimized ${_glfwBuild}/src/Debug/glfw3.lib

		debug ${_rttrBuild}/install/lib/librttr_core_d.lib			optimized ${_rttrBuild}/install/lib/librttr_core.lib
	
		debug ${_bulletBuild}/lib/Debug/BulletDynamics_Debug.lib	optimized ${_bulletBuild}/lib/Release/BulletDynamics.lib
		debug ${_bulletBuild}/lib/Debug/BulletCollision_Debug.lib	optimized ${_bulletBuild}/lib/Release/BulletCollision.lib
		debug ${_bulletBuild}/lib/Debug/LinearMath_Debug.lib		optimized ${_bulletBuild}/lib/Release/LinearMath.lib 

		debug ${_assimpDebugLib}									optimized ${_assimpReleaseLib} 
		debug ${_assimpBuild}/contrib/irrXML/Debug/IrrXMLd.lib		optimized ${_assimpBuild}/contrib/irrXML/Release/IrrXML.lib

		debug ${_alBuild}/Debug/OpenAL32.lib						optimized ${_alBuild}/Release/OpenAL32.lib

		debug ${_vcpkgInstall}/debug/lib/zlibd.lib					optimized ${_vcpkgInstall}/lib/zlib.lib
		debug ${_vcpkgInstall}/debug/lib/freetyped.lib				optimized ${_vcpkgInstall}/lib/freetype.lib	)

	if (MSVC)
		target_link_libraries(${TARGET} opengl32.lib)
	endif(MSVC)

endfunction(dependancyLinks)


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
		debug ${_dbg}atkmm.lib				optimized ${_rel}atkmm.lib	 
		debug ${_dbg}cairomm-1.0.lib		optimized ${_rel}cairomm-1.0.lib	 
		debug ${_dbg}epoxy.lib				optimized ${_rel}epoxy.lib	 
		debug ${_dbg}expat.lib				optimized ${_rel}expat.lib	 
		debug ${_dbg}fontconfig.lib			optimized ${_rel}fontconfig.lib	 
		debug ${_dbg}gailutil-3.0.lib		optimized ${_rel}gailutil-3.0.lib	 
		debug ${_dbg}gdk_pixbuf-2.0.lib		optimized ${_rel}gdk_pixbuf-2.0.lib	 
		debug ${_dbg}gdk-3.0.lib			optimized ${_rel}gdk-3.0.lib	 
		debug ${_dbg}gdkmm.lib				optimized ${_rel}gdkmm.lib	 
		debug ${_dbg}gio-2.0.lib			optimized ${_rel}gio-2.0.lib	 
		debug ${_dbg}giomm.lib				optimized ${_rel}giomm.lib	 
		debug ${_dbg}glib-2.0.lib			optimized ${_rel}glib-2.0.lib	 
		debug ${_dbg}glibmm.lib				optimized ${_rel}glibmm.lib	 
		debug ${_dbg}gmodule-2.0.lib		optimized ${_rel}gmodule-2.0.lib	 
		debug ${_dbg}gobject-2.0.lib		optimized ${_rel}gobject-2.0.lib	 
		debug ${_dbg}gthread-2.0.lib		optimized ${_rel}gthread-2.0.lib	 
		debug ${_dbg}gtk-3.0.lib			optimized ${_rel}gtk-3.0.lib	 
		debug ${_dbg}gtkmm.lib				optimized ${_rel}gtkmm.lib	 
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
function(getVcpkgLibs out_list)
	set (${out_list} "freetype" "bz2" "libpng16" "zlib" "zlibd" PARENT_SCOPE)
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
	
	set(_assimpBuild )
	getAssimpBuildDir(_assimpBuild)
	include_directories("${_assimpBuild}/include/")	
	
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/stb")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/mikkt")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/glad/gl-bindings/include")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/bullet/bullet3/src")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/openal/openal-soft/include")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/assimp/assimp/include")
	include_directories("${ENGINE_DIRECTORY_ABS}/third_party/glfw/glfw/include")

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
	getVcpkgLibs(vcpkg_libs)

	set(_alBuild )
	getOpenAlBuildDir(_alBuild)

	# where the lib files live
	set(_cfg "Release") 
	set(_vcCfg "")
	if(("$<CONFIG>" STREQUAL "Debug") OR ("$<CONFIG>" STREQUAL "DebugEditor"))
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
	endif()
	
	file(GLOB debugDlls ${_vcpkgInstall}/debug/bin/*.dll)
	getMatchingFiles("${vcpkg_libs}" "${debugDlls}" debugDlls)
	foreach(_dll ${debugDlls})
		add_custom_command(TARGET ${_target} 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugEditor>>,copy_if_different\ "${_dll}"\ $<TARGET_FILE_DIR:${_target}>,echo\ "">
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugEditor>>,echo\ "Copying ${_dll}",echo\ "">
		)
	endforeach()
	file(GLOB releaseDlls ${_vcpkgInstall}/bin/*.dll)
	getMatchingFiles("${vcpkg_libs}" "${releaseDlls}" releaseDlls)
	foreach(_dll ${releaseDlls})
		add_custom_command(TARGET ${_target} 
			POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugEditor>>,echo\ "",copy_if_different\ "${_dll}"\ $<TARGET_FILE_DIR:${_target}>>
			COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugEditor>>,echo\ "",echo\ "Copying ${_dll}">
		)
	endforeach()
	
	add_custom_command(TARGET ${_target} 
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugEditor>>,copy_if_different\ "${_alBuild}/Debug/OpenAL32.dll"\ $<TARGET_FILE_DIR:${_target}>,echo\ "">
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugEditor>>,echo\ "Copying ${_alBuild}/Debug/OpenAL32.dll",echo\ "">
	)
	add_custom_command(TARGET ${_target} 
		POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugEditor>>,echo\ "",copy_if_different\ "${_alBuild}/Release/OpenAL32.dll"\ $<TARGET_FILE_DIR:${_target}>>
		COMMAND ${CMAKE_COMMAND} -E $<IF:$<OR:$<CONFIG:Debug>,$<CONFIG:DebugEditor>>,echo\ "",echo\ "Copying ${_alBuild}/Release/OpenAL32.dll">
	)

endfunction(copyDllCommand)


# install dll (and pdb) files in the appropriate directory according to configuration
######################################################################################
function(installDlls TARGET _suffix)

	set(baseBinDir "${PROJECT_DIRECTORY}/bin")

	# paths for our libraries depend on the architecture we compile for
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(platform "x64")
	 else() 
		set(platform "x32")
	endif()
	
	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)
	set(vcpkg_libs )
	getVcpkgLibs(vcpkg_libs)

	set(_alBuild )
	getOpenAlBuildDir(_alBuild)

	foreach(configType ${CMAKE_CONFIGURATION_TYPES})

		set(binDir "${baseBinDir}/${configType}_${platform}/${TARGET}${_suffix}")

		# where the lib files live
		set(libcfg "Release") 
		set(_vcCfg "")
		if(("${configType}" STREQUAL "Debug") OR ("${configType}" STREQUAL "DebugEditor"))
			set(libcfg "Debug")
			set(_vcCfg "/debug")

			# for debug applications we also copy pdbs
			file(GLOB pdbs ${_vcpkgInstall}${_vcCfg}/bin/*.pdb)
			getMatchingFiles("${vcpkg_libs}" "${pdbs}" pdbs)
			foreach(_pdb ${pdbs})
				install(FILES ${_pdb} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
			endforeach()
			
			install(FILES ${_alBuild}/Debug/OpenAL32.pdb CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
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
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(platform "x64")
	 else() 
		set(platform "x32")
	endif()
	
	set(_vcpkgInstall )
	getVcpkgInstallDir(_vcpkgInstall)

	foreach(configType ${CMAKE_CONFIGURATION_TYPES})

		set(binDir "${baseBinDir}/${configType}_${platform}/${TARGET}/bin")

		set(_vcCfg "")
		if(("${configType}" STREQUAL "Debug") OR ("${configType}" STREQUAL "DebugEditor"))
			set(_vcCfg "/debug")

			# for debug applications we also copy pdbs
			file(GLOB pdbs ${_vcpkgInstall}${_vcCfg}/bin/*.pdb)
			foreach(_pdb ${pdbs})
				install(FILES ${_pdb} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
			endforeach()
		endif()

		# copy dlls for all libraries		
		file(GLOB dlls ${_vcpkgInstall}${_vcCfg}/bin/*.dll)
		foreach(_dll ${dlls})
			install(FILES ${_dll} CONFIGURATIONS ${configType} DESTINATION ${binDir}/)
		endforeach()

	endforeach()

endfunction(installEditorDlls)


# install everything in the appropriate directory according to configuration
#############################################################################
function(installResources TARGET)

	set(baseBinDir "${PROJECT_DIRECTORY}/bin")

	# paths for our libraries depend on the architecture we compile for
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(platform "x64")
	 else() 
		set(platform "x32")
	endif()

	foreach(configType ${CMAKE_CONFIGURATION_TYPES})

		set(binDir "${baseBinDir}/${configType}_${platform}/${TARGET}")

		# copy config files
		install(DIRECTORY ${PROJECT_DIRECTORY}/source/Runtime/Config/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/)

	endforeach()

endfunction(installResources)


# install everything in the appropriate directory according to configuration
#############################################################################
function(installEditorResources TARGET)

	set(baseBinDir "${PROJECT_DIRECTORY}/bin")

	# paths for our libraries depend on the architecture we compile for
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(platform "x64")
	 else() 
		set(platform "x32")
	endif()

	set(_edPackagedDir "${ENGINE_DIRECTORY_ABS}/third_party/gtk/GTK-for-Windows-Runtime-Environment-Installer/gtk-nsis-pack")

	foreach(configType ${CMAKE_CONFIGURATION_TYPES})

		set(binDir "${baseBinDir}/${configType}_${platform}/${TARGET}")

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
			
		# theme
		install(DIRECTORY ${ENGINE_DIRECTORY_ABS}/third_party/gtk/Ultimate-Maia/Ultimate-Maia/gtk-3.0/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/share/themes/Ultimate-Maia/gtk-3.0)
		install(DIRECTORY ${ENGINE_DIRECTORY_ABS}/third_party/gtk/Ultimate-Maia/Ultimate-Dark/gtk-3.0/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/share/themes/Ultimate-Maia/gtk-3.0)
		install(DIRECTORY ${ENGINE_DIRECTORY_ABS}/third_party/gtk/Ultimate-Maia/Ultimate-Maia/metacity-1/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/share/themes/Ultimate-Maia/metacity-1)
		install(FILES ${ENGINE_DIRECTORY_ABS}/third_party/gtk/Ultimate-Maia/Ultimate-Dark/index.theme
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/share/themes/Ultimate-Maia)

	endforeach()

endfunction(installEditorResources)


# cook package files and install them in the binary output directory
#####################################################################
function(installCookResources TARGET)

	set(cmp_dir "${PROJECT_DIRECTORY}/resources/")
	set(cmp_dir_engine "${ENGINE_DIRECTORY_ABS}/resources/")

	# figure out the directory the cooker binary lives in
	#-----------------------------------------------------------
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(_p "x64")
	 else() 
		set(_p "x32")
	endif()
	set(bin_base_dir "${PROJECT_DIRECTORY}/bin/$<CONFIG>_${_p}/")
	set(cooker_dir "${bin_base_dir}EtCooker/")
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
		DEPENDS ${deps} EtCooker 
		
		COMMAND ${CMAKE_COMMAND} -E echo "Cooking resource packages - Source ${res_file} ; Out directory: ${pak_file_dir}"
		COMMAND ${CMAKE_COMMAND} -E echo ""
		COMMAND ${CMAKE_COMMAND} -E echo "${cooker_dir}EtCooker.exe ${res_file} ${res_file_engine} ${pak_file_dir} n"
		COMMAND ${cooker_dir}EtCooker.exe ${res_file} ${res_file_engine} ${pak_file_dir} n
		COMMAND ${CMAKE_COMMAND} -E echo ""
		COMMAND ${CMAKE_COMMAND} -E echo ""
		
		COMMENT "Cooking installed resource files"

		VERBATIM
	)

endfunction(installCookResources)
