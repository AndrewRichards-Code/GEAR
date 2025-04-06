cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Input)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(Input_SRC_HEADERS
	"InputInterfaces.h"
	"InputManager.h"
)
set(Input_SRC_CPP_FILES
	"InputInterfaces.cpp"
	"InputManager.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${Input_SRC_CPP_FILES} ${Input_SRC_HEADERS})

#XInput
target_link_libraries(${TARGET_NAME} PRIVATE
	xinput.lib
)

#GLFW
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_SOURCE_DIR}/External/GLFW/include/"
)
target_link_directories(${TARGET_NAME} PRIVATE
	"${GEAR_SOURCE_DIR}/External/GLFW/lib-vc2022/"
)
target_link_libraries(${TARGET_NAME} PRIVATE
	glfw3dll.lib
)

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_INPUT_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	ARC
)