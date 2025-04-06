cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Animation)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(Animation_SRC_HEADERS
	"Animation.h"
	"Animator.h"
)
set(Animation_SRC_CPP_FILES
	"Animator.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${Animation_SRC_CPP_FILES} ${Animation_SRC_HEADERS})

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_ANIMATION_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	GEAR_CORE_Objects
	GEAR_CORE_Core
	ARC
)