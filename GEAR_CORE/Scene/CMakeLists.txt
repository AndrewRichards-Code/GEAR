cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Scene)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(Scene_SRC_HEADERS
	"Components.h"
	"Entity.h"
	"NativeScript.h"
	"NativeScriptManager.h"
	"Scene.h"
)
set(Scene_SRC_CPP_FILES
	"Components.cpp"
	"NativeScriptManager.cpp"
	"Scene.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${Scene_SRC_CPP_FILES} ${Scene_SRC_HEADERS})

#Set up post-build compile command
target_compile_definitions(${TARGET_NAME} PRIVATE "BUILD_DIR=\"${CMAKE_BINARY_DIR}\"")
target_compile_definitions(${TARGET_NAME} PRIVATE "SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_SCENE_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	GEAR_CORE_Rendering
	GEAR_CORE_Objects
	GEAR_CORE_Graphics
	GEAR_CORE_Asset
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)