cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Core)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(Core_SRC_HEADERS
	"Application.h"
	"ApplicationContext.h"
	"CommandLineOptions.h"
	"ConfigFile.h"
	"DataBuffer.h"
	"EntryPoint.h"
	"FileDialog.h"
	"Hashing.h"
	"JsonFileHelper.h"
	"ParseStack.h"
	"PlatformMacros.h"
	"Sequencer.h"
	"Timer.h"
	"TypesCppHlsl.h"
	"UUID.h"
)
set(Core_SRC_CPP_FILES
	"Application.cpp"
	"ApplicationContext.cpp"
	"DataBuffer.cpp"
	"ConfigFile.cpp"
	"FileDialog.cpp"
	"JsonFileHelper.cpp"
	"Sequencer.cpp"
	"Timer.cpp"
	"UUID.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${Core_SRC_CPP_FILES} ${Core_SRC_HEADERS})

#SRC
set(SRC_HEADERS
	"../gear_core.h"
	"../gear_core_common.h"
)
target_sources(${TARGET_NAME} PRIVATE ${SRC_CPP_FILES} ${SRC_HEADERS})
source_group("src" FILES ${SRC_CPP_FILES} ${SRC_HEADERS})


#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_CORE_BUILD_DLL)

#Set up BUILD_DIR and SOURCE_DIR
target_compile_definitions(${TARGET_NAME} PUBLIC "BUILD_DIR=\"${CMAKE_BINARY_DIR}\"")
target_compile_definitions(${TARGET_NAME} PUBLIC "SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

#Links
target_link_libraries(${TARGET_NAME} PRIVATE
	MIRU_CORE
	ARC
)