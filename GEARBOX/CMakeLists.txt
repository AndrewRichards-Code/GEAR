cmake_minimum_required(VERSION 3.27)

#GEARBOX
set(TARGET_NAME GEARBOX)
message(STATUS "${TARGET_NAME}")

#Files
set(SRC_HEADERS
	"Source/gearbox.h"
	"Source/gearbox_common.h"
)
set(SRC_CPP_FILES
	"Source/gearbox.cpp"
	"Source/gearbox_common.cpp"
)
set(CONFIG_FILES
	"Config/config.gbcf"
)
set(ASSET_REGISTRY_FILES
	"GEARBOX.gar"
)

add_executable(${TARGET_NAME})
target_sources(${TARGET_NAME} PRIVATE ${SRC_CPP_FILES} ${SRC_HEADERS} ${CONFIG_FILES} ${ASSET_REGISTRY_FILES})

source_group("Config" FILES ${CONFIG_FILES})
source_group("Source" FILES ${SRC_CPP_FILES} ${SRC_HEADERS})

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