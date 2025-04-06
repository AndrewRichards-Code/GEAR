cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Rendering)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

#Rendering
set(Rendering_SRC_HEADERS
	"DebugRender.h"
	"Pass.h"
	"PassParameters.h"
	"Renderer.h"
	"RenderGraph.h"
	"Resource.h"
)
set(Rendering_Passes_SRC_HEADERS
	"Passes/MainRenderPasses.h"
	"Passes/MipmapPasses.h"
	"Passes/OnScreenDisplayPasses.h"
	"Passes/PostProcessingPasses.h"
	"Passes/ShadowPasses.h"
	"Passes/SkyboxPasses.h"
	"Passes/SwapchinUIPasses.h"
	"Passes/TransferPasses.h"
)
set(Rendering_SRC_CPP_FILES
	"DebugRender.cpp"
	"Pass.cpp"
	"PassParameters.cpp"
	"Renderer.cpp"
	"RenderGraph.cpp"
	"Resource.cpp"
)
set(Rendering_Passes_SRC_CPP_FILES
	"Passes/MainRenderPasses.cpp"
	"Passes/MipmapPasses.cpp"
	"Passes/OnScreenDisplayPasses.cpp"
	"Passes/PostProcessingPasses.cpp"
	"Passes/ShadowPasses.cpp"
	"Passes/SkyboxPasses.cpp"
	"Passes/SwapchinUIPasses.cpp"
	"Passes/TransferPasses.cpp"
)

target_sources(${TARGET_NAME} PRIVATE ${Rendering_SRC_CPP_FILES} ${Rendering_SRC_HEADERS})

target_sources(${TARGET_NAME} PRIVATE ${Rendering_Passes_SRC_CPP_FILES} ${Rendering_Passes_SRC_HEADERS})
source_group("Passes" FILES ${Rendering_Passes_SRC_CPP_FILES} ${Rendering_Passes_SRC_HEADERS})

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

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_RENDERING_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	GEAR_CORE_Objects
	GEAR_CORE_Graphics
	GEAR_CORE_Asset
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)