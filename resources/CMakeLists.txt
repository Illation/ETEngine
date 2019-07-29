

#####################
# Resource CMake file
#####################


# resources generating compiled source files
############################################

set(gen_dir "${CMAKE_CURRENT_SOURCE_DIR}/../source/Demo/_generated/")
set(cmp_dir "${CMAKE_CURRENT_SOURCE_DIR}/")

# figure out the directory the cooker binary lives in
if("${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
	set(_p "x64")
 else() 
	set(_p "x32")
endif()
set(cooker_dir "${CMAKE_CURRENT_SOURCE_DIR}/../bin/$<CONFIG>_${_p}/EtCooker/")

set(resource_name "compiledPackage")
set(target_files "${gen_dir}${resource_name}.h" "${gen_dir}${resource_name}.cpp")
set(res_file "${cmp_dir}asset_database.json")

# any files that can trigger the resources to be rebuilt
file(GLOB_RECURSE deps ${cmp_dir}/*.*)

# the command list that will run - create dir, set env var, run compile source, log stuff
add_custom_command(
    OUTPUT ${target_files}
    DEPENDS ${deps} EtCooker

	COMMAND ${CMAKE_COMMAND} -E echo "Cooking resource packages - Source Directory: ${cmp_dir} ; Generated Directory: ${gen_dir}"
	COMMAND ${CMAKE_COMMAND} -E echo ""
    COMMAND ${cooker_dir}EtCooker.exe ${res_file} ${gen_dir} ${resource_name}
	COMMAND ${CMAKE_COMMAND} -E echo ""
	COMMAND ${CMAKE_COMMAND} -E echo ""
	
    COMMENT "Generating resource source file"

    VERBATIM
)

add_custom_target( run-cooker DEPENDS ${target_files} )

foreach(_target_file ${target_files})
	set_source_files_properties(${_target_file} PROPERTIES GENERATED TRUE )
endforeach()
