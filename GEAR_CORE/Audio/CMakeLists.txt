cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Audio)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(Audio_SRC_HEADERS
	"AudioInterfaces.h"
	"AudioListener.h"
	"AudioSource.h"
	"AudioStreaming.h"
)
set(Audio_SRC_CPP_FILES
	"AudioInterfaces.cpp"
	"AudioListener.cpp"
	"AudioSource.cpp"
	"AudioStreaming.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${Audio_SRC_CPP_FILES} ${Audio_SRC_HEADERS})

#XAudio2
target_link_libraries(${TARGET_NAME} PRIVATE
	xaudio2.lib
)

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_AUDIO_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	GEAR_CORE_Objects
	ARC
)