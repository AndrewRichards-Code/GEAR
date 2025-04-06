cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_ImGui)
message(STATUS "${TARGET_NAME}")

add_library(${TARGET_NAME} STATIC)

#ImGui
set(IMGUI_HEADERS
  "${GEAR_SOURCE_DIR}/External/imgui/backends/imgui_impl_dx12.h"
  "${GEAR_SOURCE_DIR}/External/imgui/backends/imgui_impl_glfw.h"
  "${GEAR_SOURCE_DIR}/External/imgui/backends/imgui_impl_vulkan.h"
  "${GEAR_SOURCE_DIR}/External/imgui/imconfig.h"
  "${GEAR_SOURCE_DIR}/External/imgui/imgui.h"
  "${GEAR_SOURCE_DIR}/External/imgui/imgui_internal.h"
  "${GEAR_SOURCE_DIR}/External/imgui/imstb_rectpack.h"
  "${GEAR_SOURCE_DIR}/External/imgui/imstb_textedit.h"
  "${GEAR_SOURCE_DIR}/External/imgui/imstb_truetype.h"
  "${GEAR_SOURCE_DIR}/External/imgui/misc/cpp/imgui_stdlib.h"
)
set(IMGUI_CPP_FILES
  "${GEAR_SOURCE_DIR}/External/imgui/backends/imgui_impl_dx12.cpp"
  "${GEAR_SOURCE_DIR}/External/imgui/backends/imgui_impl_glfw.cpp"
  "${GEAR_SOURCE_DIR}/External/imgui/backends/imgui_impl_vulkan.cpp"
  "${GEAR_SOURCE_DIR}/External/imgui/imgui.cpp"
  "${GEAR_SOURCE_DIR}/External/imgui/imgui_demo.cpp"
  "${GEAR_SOURCE_DIR}/External/imgui/imgui_draw.cpp"
  "${GEAR_SOURCE_DIR}/External/imgui/imgui_tables.cpp"
  "${GEAR_SOURCE_DIR}/External/imgui/imgui_widgets.cpp"
  "${GEAR_SOURCE_DIR}/External/imgui/misc/cpp/imgui_stdlib.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${IMGUI_CPP_FILES} ${IMGUI_HEADERS})
source_group("ImGui" FILES ${IMGUI_CPP_FILES} ${IMGUI_HEADERS})

#ImGuizmo
set(IMGUIZMO_HEADERS
  "${GEAR_SOURCE_DIR}/External/ImGuizmo/ImGuizmo.h"
  "${GEAR_SOURCE_DIR}/External/ImGuizmo/GraphEditor.h"
)
set(IMGUIZMO_CPP_FILES
  "${GEAR_SOURCE_DIR}/External/ImGuizmo/ImGuizmo.cpp"
  "${GEAR_SOURCE_DIR}/External/ImGuizmo/GraphEditor.cpp"
)
target_sources(${TARGET_NAME} PRIVATE ${IMGUIZMO_CPP_FILES} ${IMGUIZMO_HEADERS})
source_group("ImGuizmo" FILES ${IMGUIZMO_CPP_FILES} ${IMGUIZMO_HEADERS})

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
	target_include_directories(${TARGET_NAME} PRIVATE ${Vulkan_INCLUDE_DIRS})
	target_link_directories(${TARGET_NAME} PRIVATE "${Vulkan_SDK_DIR}/Lib")
	target_link_libraries(${TARGET_NAME} PRIVATE vulkan-1.lib)
endif()

#IMGUI
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_SOURCE_DIR}/External/imgui/"
)

#Includes
target_include_directories(${TARGET_NAME} PRIVATE 
	"${GEAR_CORE_SOURCE_DIR}"
	"${GEAR_SOURCE_DIR}/External"
	"${GEAR_SOURCE_DIR}/External/MIRU/External"
)

#Defines
target_compile_definitions(${TARGET_NAME} PRIVATE GEAR_IMGUI_BUILD_DLL)

#Links
target_link_libraries(${TARGET_NAME} PRIVATE 
	ARC
)