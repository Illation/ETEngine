

####################################
# Dear ImGui - Debug UI rendering
####################################


set (sources 
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp)
	
set (headers 
	${CMAKE_CURRENT_SOURCE_DIR}/imconfig.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h)

list (APPEND projectFiles ${sources} ${headers})

target_definitions()

message(STATUS "Adding target: imgui")
add_library(imgui ${projectFiles})
targetCompileOptions(imgui)
target_compile_definitions(imgui PRIVATE "-DIMGUI_USER_CONFIG=<imconfig.h>")

assign_source_group(${projectFiles})
assignIdeFolder(imgui Engine/3rdParty)
libOutputDirectories(imgui)

target_link_libraries (imgui EtMath EtCore EtRendering NothingsStb)

include_directories("${ENGINE_SOURCE_DIR}")
target_include_directories (imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
