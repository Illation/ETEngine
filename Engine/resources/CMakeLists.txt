

#####################
# Resource CMake file
#####################


# resources generating compiled source files
############################################

set(gen_dir "${PROJECT_DIRECTORY}/source/Runtime/_generated/")
set(cmp_dir "${CMAKE_CURRENT_SOURCE_DIR}/")

# figure out the directory the cooker binary lives in
#-----------------------------------------------------------
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	set(_p "x64")
 else() 
	set(_p "x32")
endif()
set(bin_base_dir "${PROJECT_DIRECTORY}/bin/$<CONFIG>_${_p}/")
set(cooker_dir "${bin_base_dir}EtCooker/")

set(resource_name "compiled_package")
set(target_files "${gen_dir}${resource_name}.h" "${gen_dir}${resource_name}.cpp")
set(res_file "${cmp_dir}asset_database.json")

# any files that can trigger the resources to be rebuilt
file(GLOB_RECURSE deps ${cmp_dir}/assets/*.*)
list (APPEND deps ${cmp_dir}/asset_database.json)


# the command list that will run - for compiling resources
#-----------------------------------------------------------
add_custom_command(
    OUTPUT ${target_files}
    DEPENDS ${deps} EtCooker

	COMMAND ${CMAKE_COMMAND} -E echo "Cooking resource packages - Source ${res_file} ; Generated Directory: ${gen_dir}"
	COMMAND ${CMAKE_COMMAND} -E echo ""
	COMMAND ${CMAKE_COMMAND} -E echo "${cooker_dir}EtCooker.exe ${res_file} ${gen_dir} y ${resource_name}"
    COMMAND ${cooker_dir}EtCooker.exe ${res_file} ${gen_dir} y ${resource_name}
	COMMAND ${CMAKE_COMMAND} -E echo ""
	COMMAND ${CMAKE_COMMAND} -E echo ""
	
    COMMENT "Generating resource source file"

    VERBATIM
)

add_custom_target( cook-compiled-resources DEPENDS ${target_files} )

foreach(_target_file ${target_files})
	get_filename_component(_abs_target "${_target_file}" ABSOLUTE)
	set_source_files_properties(${_abs_target} PROPERTIES GENERATED TRUE )
endforeach()