cmake_minimum_required(VERSION 3.27)

#GEAR_CORE_Asset
set(TARGET_NAME GEAR_CORE_UI)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} SHARED)

set(UI_SRC_HEADERS
	"MenuBar.h"
	"UIContext.h"
)
set(UI_Panels_SRC_HEADERS
	"Panels/BasePanel.h"
	"Panels/ContentBrowserPanel.h"
	"Panels/ContentEditorPanel.h"
	"Panels/MaterialPanel.h"
	"Panels/OutputPanel.h"
	"Panels/Panels.h"
	"Panels/ProjectPanel.h"
	"Panels/PropertiesPanel.h"
	"Panels/RendererPropertiesPanel.h"
	"Panels/SceneHierarchyPanel.h"
	"Panels/ViewportPanel.h"
)
set(UI_ComponentUI_SRC_HEADERS
	"ComponentUI/CameraComponentUI.h"
	"ComponentUI/ComponentUI.h"
	"ComponentUI/ComponentUIs.h"
	"ComponentUI/LightComponentUI.h"
	"ComponentUI/MaterialComponentUI.h"
	"ComponentUI/ModelComponentUI.h"
	"ComponentUI/NameComponentUI.h"
	"ComponentUI/SkyboxComponentUI.h"
	"ComponentUI/TextureComponentUI.h"
	"ComponentUI/TransformComponentUI.h"
)
set(UI_SRC_CPP_FILES
	"MenuBar.cpp"
	"UIContext.cpp"
)
set(UI_Panels_SRC_CPP_FILES
	"Panels/ContentBrowserPanel.cpp"
	"Panels/ContentEditorPanel.cpp"
	"Panels/MaterialPanel.cpp"
	"Panels/OutputPanel.cpp"
	"Panels/ProjectPanel.cpp"
	"Panels/PropertiesPanel.cpp"
	"Panels/RendererPropertiesPanel.cpp"
	"Panels/SceneHierarchyPanel.cpp"
	"Panels/ViewportPanel.cpp"
)
set(UI_ComponentUI_SRC_CPP_FILES
	"ComponentUI/CameraComponentUI.cpp"
	"ComponentUI/ComponentUI.cpp"
	"ComponentUI/LightComponentUI.cpp"
	"ComponentUI/MaterialComponentUI.cpp"
	"ComponentUI/ModelComponentUI.cpp"
	"ComponentUI/NameComponentUI.cpp"
	"ComponentUI/SkyboxComponentUI.cpp"
	"ComponentUI/TextureComponentUI.cpp"
	"ComponentUI/TransformComponentUI.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${UI_SRC_CPP_FILES} ${UI_SRC_HEADERS})

target_sources(${TARGET_NAME} PRIVATE ${UI_Panels_SRC_CPP_FILES} ${UI_Panels_SRC_HEADERS})
source_group("Panels" FILES ${UI_Panels_SRC_CPP_FILES} ${UI_Panels_SRC_HEADERS})

target_sources(${TARGET_NAME} PRIVATE ${UI_ComponentUI_SRC_CPP_FILES} ${UI_ComponentUI_SRC_HEADERS})
source_group("ComponentUI" FILES ${UI_ComponentUI_SRC_CPP_FILES} ${UI_ComponentUI_SRC_HEADERS})

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
	file(GLOB GEAR_MIRU_D3D12_PACKAGES "${CMAKE_BINARY_DIR}/packages/*")
	target_include_directories(${TARGET_NAME} PRIVATE "${GEAR_MIRU_D3D12_PACKAGES}")
	target_link_libraries(${TARGET_NAME} PRIVATE d3d12.lib dxgi.lib)
endif()
find_package(Vulkan)
if (Vulkan_FOUND)
	set(Vulkan_SDK_DIR "${Vulkan_INCLUDE_DIRS}/../")
	target_include_directories(${TARGET_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
	target_link_directories(${TARGET_NAME} PRIVATE "${Vulkan_SDK_DIR}/Lib")
	target_link_libraries(${TARGET_NAME} PRIVATE vulkan-1.lib)
endif()

#YAML-CPP
target_link_libraries(${TARGET_NAME} PRIVATE
	yaml-cpp::yaml-cpp
)

#IMGUI
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_SOURCE_DIR}/External/imgui/"
)

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
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_UI_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	GEAR_CORE_Project
	GEAR_CORE_Scene
	GEAR_CORE_ImGui
	GEAR_CORE_AssetManager
	GEAR_CORE_Rendering
	GEAR_CORE_Objects
	GEAR_CORE_Graphics
	GEAR_CORE_Asset
	GEAR_CORE_Core
	MIRU_CORE
	ARC
)