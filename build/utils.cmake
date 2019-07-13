

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
	set(CMAKE_CXX_FLAGS_DEBUGEDITOR "${CMAKE_CXX_FLAGS_DEBUG} -DEDITOR" )
	set(CMAKE_C_FLAGS_DEBUGEDITOR "${CMAKE_C_FLAGS_DEBUG} -DEDITOR")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUGEDITOR "${CMAKE_EXE_LINKER_FLAGS_DEBUG} -DEDITOR")

	set(CMAKE_CXX_FLAGS_DEVELOP "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
	set(CMAKE_C_FLAGS_DEVELOP "${CMAKE_C_FLAGS_RELWITHDEBINFO}")
	set(CMAKE_EXE_LINKER_FLAGS_DEVELOP "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO}")

	set(CMAKE_CXX_FLAGS_DEVELOPEDITOR "${CMAKE_CXX_FLAGS_DEVELOP} -DEDITOR")
	set(CMAKE_C_FLAGS_DEVELOPEDITOR "${CMAKE_C_FLAGS_DEVELOP} -DEDITOR")
	set(CMAKE_EXE_LINKER_FLAGS_DEVELOPEDITOR "${CMAKE_EXE_LINKER_FLAGS_DEVELOP} -DEDITOR")

	set(CMAKE_CXX_FLAGS_SHIPPING "${CMAKE_CXX_FLAGS_RELEASE}")
	set(CMAKE_C_FLAGS_SHIPPING "${CMAKE_C_FLAGS_RELEASE}" )
	set(CMAKE_EXE_LINKER_FLAGS_SHIPPING "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

endmacro(setupConfigurations)


# output dir for executables
############################
function(outputDirectories TARGET)

	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8") # 64 bit
		set(_p "x64")	
	else() # 32 bit
		set(_p "x32")	
	endif()

	foreach(_c ${CMAKE_CONFIGURATION_TYPES})
		string(TOUPPER ${_c} _C)

		set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${_C} ${PROJECT_BINARY_DIR}/../bin/${_c}_${_p}/${TARGET})
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

		set_target_properties(${TARGET} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${_C} ${PROJECT_BINARY_DIR}/../lib/${_c}_${_p}/${TARGET})
		set_target_properties(${TARGET} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${_C} ${PROJECT_BINARY_DIR}/../lib/${_c}_${_p}/${TARGET})
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
			set_source_files_properties( ${src_file} PROPERTIES COMPILE_FLAGS "/Yustdafx.hpp" )
		endforeach( src_file ${SOURCELIST} )
		# run this second to overwrite the pch setting for the stdafx.cpp file
		set_source_files_properties(stdafx.cpp PROPERTIES COMPILE_FLAGS "/Ycstdafx.hpp" )
	endif(MSVC)
endfunction(precompiled_headers)


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


# link to all dependancies
###########################
function(dependancyLinks TARGET _useSdlMain)

	set(dep_dir "${PROJECT_BINARY_DIR}/../dependancies")

	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(dep_pf "${dep_dir}/x64")
	 else() 
		set(dep_pf "${dep_dir}/x32")
	endif()

	# separate debug and release libs
	target_link_libraries (${TARGET} 		
		debug ${dep_pf}/rttr/Debug/rttr_core_d.lib			optimized ${dep_pf}/rttr/Release/rttr_core.lib
	
		debug ${dep_pf}/bullet/Debug/BulletDynamics.lib		optimized ${dep_pf}/bullet/Release/BulletDynamics.lib
		debug ${dep_pf}/bullet/Debug/BulletCollision.lib	optimized ${dep_pf}/bullet/Release/BulletCollision.lib
		debug ${dep_pf}/bullet/Debug/LinearMath.lib			optimized ${dep_pf}/bullet/Release/LinearMath.lib )

	target_link_libraries (${TARGET} 
		${dep_pf}/sdl2/SDL2.lib
		${dep_pf}/freeImage/FreeImage.lib
		${dep_pf}/freetype/freetype.lib
		${dep_pf}/assimp/assimp.lib
		${dep_pf}/openAL/openAL.lib )

	if (MSVC)
		target_link_libraries(${TARGET} opengl32.lib)

		if (_useSdlMain)
			target_link_libraries(${TARGET} ${dep_pf}/sdl2/SDL2main.lib)
		endif(_useSdlMain)

	endif(MSVC)

endfunction(dependancyLinks)


# link to all dependancies
###########################
function(libIncludeDirs)

	list (APPEND libs "sdl2" "freeImage" "freetype" "assimp" "openAL" "rttr" "bullet")

	foreach(_lib ${libs})
		include_directories("${PROJECT_BINARY_DIR}/../dependancies/include/${_lib}/")	
	endforeach(_lib)

endfunction(libIncludeDirs)


# install everything in the appropriate directory according to configuration
###########################
function(installResources TARGET)

	set(projectBase "${PROJECT_BINARY_DIR}/..")
	set(baseBinDir "${projectBase}/bin")

	# paths for our libraries depend on the architecture we compile fo
	if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
		set(platform "x64")
	 else() 
		set(platform "x32")
	endif()

	list (APPEND uni_libs "sdl2" "freeImage" "freetype" "assimp" "openAL")
	list (APPEND sep_libs "bullet" "rttr")

	foreach(configType ${CMAKE_CONFIGURATION_TYPES})

		set(binDir "${baseBinDir}/${configType}_${platform}/${TARGET}")

		# where the lib files live
		set(libcfg "Release") 
		if(("${configType}" STREQUAL "Debug") OR ("${configType}" STREQUAL "DebugEditor"))
			set(libcfg "Debug")

			# for debug applications we also copy pdbs
			foreach(_lib ${uni_libs})
				install(DIRECTORY ${projectBase}/dependancies/${platform}/${_lib}/
					CONFIGURATIONS ${configType}
					DESTINATION ${binDir}/
					FILES_MATCHING PATTERN "*.pdb")
			endforeach()
			foreach(_lib ${sep_libs})
				install(DIRECTORY ${projectBase}/dependancies/${platform}/${_lib}/${libcfg}/
					CONFIGURATIONS ${configType}
					DESTINATION ${binDir}/
					FILES_MATCHING PATTERN "*.pdb")
			endforeach()
		endif()

		# copy dlls for all libraries
		foreach(_lib ${uni_libs})
			install(DIRECTORY ${projectBase}/dependancies/${platform}/${_lib}/
				CONFIGURATIONS ${configType}
				DESTINATION ${binDir}/
				FILES_MATCHING PATTERN "*.dll")
		endforeach()
		foreach(_lib ${sep_libs})
			install(DIRECTORY ${projectBase}/dependancies/${platform}/${_lib}/${libcfg}/
				CONFIGURATIONS ${configType}
				DESTINATION ${binDir}/
				FILES_MATCHING PATTERN "*.dll")
		endforeach()

		# copy packaged resources
		install(DIRECTORY ${projectBase}/source/Engine/Resources/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/Resources/)
		install(DIRECTORY ${projectBase}/source/Demo/Resources/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/Resources/)

		install(DIRECTORY ${projectBase}/source/Engine/Shaders/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/Shaders/)
		install(DIRECTORY ${projectBase}/source/Demo/Shaders/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/Shaders/)

		install(DIRECTORY ${projectBase}/source/Demo/Config/
			CONFIGURATIONS ${configType}
			DESTINATION ${binDir}/)

	endforeach()

endfunction(installResources)
