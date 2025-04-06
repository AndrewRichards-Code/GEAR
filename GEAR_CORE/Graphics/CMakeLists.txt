cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Graphics)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

#Graphics
set(Graphics_SRC_HEADERS
	"AllocatorManager.h"
	"Colour.h"
	"ColourFunctions.h"
	"Indexbuffer.h"
	"RenderPipeline.h"
	"RenderSurface.h"
	"Storagebuffer.h"
	"Texture.h"
	"Uniformbuffer.h"
	"UniformBufferStructures.h"
	"Vertexbuffer.h"
	"Window.h"
)

set(Graphics_SRC_CPP_FILES
	"AllocatorManager.cpp"
	"Colour.cpp"
	"Indexbuffer.cpp"
	"RenderPipeline.cpp"
	"RenderSurface.cpp"
	"Storagebuffer.cpp"
	"Texture.cpp"
	"Uniformbuffer.cpp"
	"Vertexbuffer.cpp"
	"Window.cpp"
)

target_sources(${TARGET_NAME} PRIVATE ${Graphics_SRC_CPP_FILES} ${Graphics_SRC_HEADERS})

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
CopyToBinaryDirPostBuild(${TARGET_NAME} "${GEAR_SOURCE_DIR}/External/GLFW/lib-vc2022/glfw3.dll" "")

#MIRU - D3D12 and Vulkan
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_SOURCE_DIR}/External/MIRU/MIRU_CORE/src/"
)
if (WIN32)
	target_link_libraries(${TARGET_NAME} PRIVATE d3d12.lib dxgi.lib)
endif()
find_package(Vulkan)
if (Vulkan_FOUND)
	set(Vulkan_SDK_DIR "${Vulkan_INCLUDE_DIRS}/../")
	target_link_directories(${TARGET_NAME} PRIVATE "${Vulkan_SDK_DIR}/Lib")
	target_link_libraries(${TARGET_NAME} PRIVATE vulkan-1.lib)
endif()

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
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_GRAPHICS_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	GEAR_CORE_Asset
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)