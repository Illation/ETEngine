

#####################
# Resource CMake file
#####################


# resources generating compiled source files
############################################

set(gen_dir "${PROJECT_DIRECTORY}/source/Runtime/_generated/")
set(cmp_dir "${PROJECT_DIRECTORY}/resources/")
set(cmp_dir_engine "${CMAKE_CURRENT_SOURCE_DIR}/")

# figure out the directory the cooker binary lives in
#-----------------------------------------------------------
set(_p )
getPlatformArch(_p)
	
set(bin_base_dir "${PROJECT_DIRECTORY}/bin/$<CONFIG>_${_p}/")
set(cooker_dir "${bin_base_dir}EtCooker/")

set(resource_name "compiled_package")
set(target_files "${gen_dir}${resource_name}.h" "${gen_dir}${resource_name}.cpp")
set(res_file "${cmp_dir}asset_database.json")
set(res_file_engine "${cmp_dir_engine}asset_database.json")

# any files that can trigger the resources to be rebuilt
file(GLOB_RECURSE deps ${cmp_dir}/assets/*.* ${cmp_dir_engine}/assets/*.*)
list (APPEND deps ${res_file})
list (APPEND deps ${res_file_engine})


# the command list that will run - for compiling resources
#-----------------------------------------------------------
add_custom_command(
    OUTPUT ${target_files}
    DEPENDS ${deps} EtCooker

	COMMAND ${CMAKE_COMMAND} -E echo "Cooking resource packages - Source ${res_file} Engine Source ${res_file_engine} ; Generated Directory: ${gen_dir}"
	COMMAND ${CMAKE_COMMAND} -E echo ""
	COMMAND ${CMAKE_COMMAND} -E echo "${cooker_dir}EtCooker.exe ${res_file} ${res_file_engine} ${gen_dir} y ${resource_name}"
    COMMAND ${cooker_dir}EtCooker.exe ${res_file} ${res_file_engine} ${gen_dir} y ${resource_name}
	COMMAND ${CMAKE_COMMAND} -E echo ""
	COMMAND ${CMAKE_COMMAND} -E echo ""
	
    COMMENT "Generating resource source file"

    VERBATIM
)

message(STATUS "Adding target: cook-compiled-resources")
add_custom_target( cook-compiled-resources DEPENDS ${target_files} )
assignIdeFolder(cook-compiled-resources Engine/Build)

foreach(_target_file ${target_files})
	get_filename_component(_abs_target "${_target_file}" ABSOLUTE)
	set_source_files_properties(${_abs_target} PROPERTIES GENERATED TRUE )
endforeach()