cmake_minimum_required(VERSION 3.27)

message(STATUS "GEARBOX")

#Files
set(SRC_HEADERS
	"src/gearbox.h"
	"src/gearbox_common.h"
	)
	set(SRC_CPP_FILES
	"src/gearbox.cpp"
	"src/gearbox_common.cpp"
)
set(OTHER_FILES
	config.gbcf
	imgui.ini
)

add_executable(GEARBOX)
target_sources(GEARBOX PRIVATE ${SRC_CPP_FILES} ${SRC_HEADERS} ${OTHER_FILES})

target_link_libraries(GEARBOX PRIVATE GEAR_CORE ARC)

source_group("src" FILES ${SRC_CPP_FILES} ${SRC_HEADERS})

target_include_directories(GEARBOX PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/../GEAR_CORE/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/MIRU/External/"
)

set_target_properties(GEARBOX PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")