cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Objects)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(Objects_SRC_HEADERS
	"Camera.h"
	"Font.h"
	"Frustum.h"
	"Light.h"
	"Material.h"
	"Mesh.h"
	"Model.h"
	"ObjectInterface.h"
	"Picker.h"
	"Probe.h"
	"Skybox.h"
	"Text.h"
	"Transform.h"
)
set(Objects_SRC_CPP_FILES
	"Camera.cpp"
	"Font.cpp"
	"Frustum.cpp"
	"Light.cpp"
	"Material.cpp"
	"Mesh.cpp"
	"Model.cpp"
	"ObjectInterface.cpp"
	"Picker.cpp"
	"Probe.cpp"
	"Skybox.cpp"
	"Text.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${Objects_SRC_CPP_FILES} ${Objects_SRC_HEADERS})

#FREETYPE
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_SOURCE_DIR}/External/FREETYPE/include/"
)
target_link_directories(${TARGET_NAME} PRIVATE
	"${GEAR_SOURCE_DIR}/External/FREETYPE/release dll/win64"
)
target_link_libraries(${TARGET_NAME} PRIVATE
	freetype.lib
)

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_OBJECTS_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	GEAR_CORE_Graphics
	GEAR_CORE_Asset
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)