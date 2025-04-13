cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Project)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(Project_SRC_HEADERS
	"Project.h"
)
set(Project_SRC_CPP_FILES
	"Project.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${Project_SRC_CPP_FILES} ${Project_SRC_HEADERS})

#YAML-CPP
target_link_libraries(${TARGET_NAME} PRIVATE
	yaml-cpp::yaml-cpp
)

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_PROJECT_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE
	GEAR_CORE_Scripting
	GEAR_CORE_Scene
	GEAR_CORE_AssetManager
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)