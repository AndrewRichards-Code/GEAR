cmake_minimum_required(VERSION 3.27)

#GEAR_CORE_Asset
set(TARGET_NAME GEARBOX)
message(STATUS "${TARGET_NAME}")

#Files
set(SRC_HEADERS
	"src/GearBox.h"
	"src/GearBox_common.h"
	)
	set(SRC_CPP_FILES
	"src/gearbox.cpp"
	"src/gearbox_common.cpp"
)
set(OTHER_FILES
	config.gbcf
	imgui.ini
)

add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${SRC_CPP_FILES} ${SRC_HEADERS} ${OTHER_FILES})

source_group("src" FILES ${SRC_CPP_FILES} ${SRC_HEADERS})

target_include_directories(${TARGET_NAME} PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/src"
	"${GEAR_SOURCE_DIR}/GEAR_CORE"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	GEAR_CORE_Graphics
	GEAR_CORE_UI
	GEAR_CORE_Project
	GEAR_CORE_Scene
	GEAR_CORE_AssetManager
	GEAR_CORE_Rendering
	GEAR_CORE_Objects
	GEAR_CORE_Asset
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)

set_target_properties(${TARGET_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set_target_properties(${TARGET_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)