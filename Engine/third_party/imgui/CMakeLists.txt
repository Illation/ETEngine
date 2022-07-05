

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
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_internal.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imconfig.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_rectpack.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_textedit.h
	${CMAKE_CURRENT_SOURCE_DIR}/imgui/imstb_truetype.h )

list (APPEND projectFiles ${sources} ${headers})

message(STATUS "Adding target: imgui")
add_library(imgui ${projectFiles})
targetCompileOptions(imgui)

assign_source_group(${projectFiles})
assignIdeFolder(imgui Engine/3rdParty)
libOutputDirectories(imgui)

target_include_directories (imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
