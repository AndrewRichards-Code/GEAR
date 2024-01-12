cmake_minimum_required(VERSION 3.27)

message(STATUS "GEAR_CORE")

add_library(GEAR_CORE SHARED)

#SRC
set(SRC_HEADERS
	"src/gear_core.h"
	"src/gear_core_common.h"
)
set(SRC_CPP_FILES
	"src/gear_core_common.cpp"
)
target_sources(GEAR_CORE PRIVATE ${SRC_CPP_FILES} ${SRC_HEADERS})
source_group("src" FILES ${SRC_CPP_FILES} ${SRC_HEADERS})

#Animation
set(Animation_SRC_HEADERS
	"src/Animation/Animation.h"
	"src/Animation/Animator.h"
)
set(Animation_SRC_CPP_FILES
	"src/Animation/Animator.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Animation_SRC_CPP_FILES} ${Animation_SRC_HEADERS})
source_group("src\\Animation" FILES ${Animation_SRC_CPP_FILES} ${Animation_SRC_HEADERS})

#Audio
set(Audio_SRC_HEADERS
	"src/Audio/AudioInterfaces.h"
	"src/Audio/AudioListener.h"
	"src/Audio/AudioSource.h"
)
set(Audio_SRC_CPP_FILES
	"src/Audio/AudioInterfaces.cpp"
	"src/Audio/AudioListener.cpp"
	"src/Audio/AudioSource.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Audio_SRC_CPP_FILES} ${Audio_SRC_HEADERS})
source_group("src\\Audio" FILES ${Audio_SRC_CPP_FILES} ${Audio_SRC_HEADERS})

#Build
set(Build_SRC_HEADERS
	"src/Build/Project.h"
)
set(Build_SRC_CPP_FILES
	"src/Build/Project.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Build_SRC_CPP_FILES} ${Build_SRC_HEADERS})
source_group("src\\Build" FILES ${Build_SRC_CPP_FILES} ${Build_SRC_HEADERS})

#Core
set(Core_SRC_HEADERS
	"src/Core/Application.h"
	"src/Core/ApplicationContext.h"
	"src/Core/AssetFile.h"
	"src/Core/CommandLineOptions.h"
	"src/Core/ConfigFile.h"
	"src/Core/EntryPoint.h"
	"src/Core/FileDialog.h"
	"src/Core/FontLibrary.h"
	"src/Core/Hashing.h"
	"src/Core/JsonFileHelper.h"
	"src/Core/ParseStack.h"
	"src/Core/PlatformMacros.h"
	"src/Core/Sequencer.h"
	"src/Core/Timer.h"
	"src/Core/TypesCppHlsl.h"
	"src/Core/UUID.h"
)
set(Core_SRC_CPP_FILES
	"src/Core/Application.cpp"
	"src/Core/ApplicationContext.cpp"
	"src/Core/AssetFile.cpp"
	"src/Core/ConfigFile.cpp"
	"src/Core/FileDialog.cpp"
	"src/Core/FontLibrary.cpp"
	"src/Core/JsonFileHelper.cpp"
	"src/Core/Timer.cpp"
	"src/Core/UUID.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Core_SRC_CPP_FILES} ${Core_SRC_HEADERS})
source_group("src\\Core" FILES ${Core_SRC_CPP_FILES} ${Core_SRC_HEADERS})

#Graphics
set(Graphics_SRC_HEADERS
	"src/Graphics/AllocatorManager.h"
	"src/Graphics/Colour.h"
	"src/Graphics/ColourFunctions.h"
	"src/Graphics/DebugRender.h"
	"src/Graphics/Indexbuffer.h"
	"src/Graphics/Picker.h"
	"src/Graphics/RenderPipeline.h"
	"src/Graphics/RenderSurface.h"
	"src/Graphics/Storagebuffer.h"
	"src/Graphics/Texture.h"
	"src/Graphics/Uniformbuffer.h"
	"src/Graphics/UniformBufferStructures.h"
	"src/Graphics/Vertexbuffer.h"
	"src/Graphics/Window.h"
)
set(Graphics_Rendering_SRC_HEADERS
	"src/Graphics/Rendering/Pass.h"
	"src/Graphics/Rendering/PassParameters.h"
	"src/Graphics/Rendering/Renderer.h"
	"src/Graphics/Rendering/RenderGraph.h"
	"src/Graphics/Rendering/Resource.h"
)
set(Graphics_Rendering_Passes_SRC_HEADERS
	"src/Graphics/Rendering/Passes/MainRenderPasses.h"
	"src/Graphics/Rendering/Passes/MipmapPasses.h"
	"src/Graphics/Rendering/Passes/OnScreenDisplayPasses.h"
	"src/Graphics/Rendering/Passes/PostProcessingPasses.h"
	"src/Graphics/Rendering/Passes/ShadowPasses.h"
	"src/Graphics/Rendering/Passes/SkyboxPasses.h"
	"src/Graphics/Rendering/Passes/SwapchinUIPasses.h"
	"src/Graphics/Rendering/Passes/TransferPasses.h"
)
set(Graphics_SRC_CPP_FILES
	"src/Graphics/AllocatorManager.cpp"
	"src/Graphics/Colour.cpp"
	"src/Graphics/DebugRender.cpp"
	"src/Graphics/Indexbuffer.cpp"
	"src/Graphics/Picker.cpp"
	"src/Graphics/RenderPipeline.cpp"
	"src/Graphics/RenderSurface.cpp"
	"src/Graphics/Texture.cpp"
	"src/Graphics/Vertexbuffer.cpp"
	"src/Graphics/Window.cpp"
)
set(Graphics_Rendering_SRC_CPP_FILES
	"src/Graphics/Rendering/Pass.cpp"
	"src/Graphics/Rendering/PassParameters.cpp"
	"src/Graphics/Rendering/Renderer.cpp"
	"src/Graphics/Rendering/RenderGraph.cpp"
	"src/Graphics/Rendering/Resource.cpp"
)
set(Graphics_Rendering_Passes_SRC_CPP_FILES
	"src/Graphics/Rendering/Passes/MainRenderPasses.cpp"
	"src/Graphics/Rendering/Passes/MipmapPasses.cpp"
	"src/Graphics/Rendering/Passes/OnScreenDisplayPasses.cpp"
	"src/Graphics/Rendering/Passes/PostProcessingPasses.cpp"
	"src/Graphics/Rendering/Passes/ShadowPasses.cpp"
	"src/Graphics/Rendering/Passes/SkyboxPasses.cpp"
	"src/Graphics/Rendering/Passes/SwapchinUIPasses.cpp"
	"src/Graphics/Rendering/Passes/TransferPasses.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Graphics_SRC_CPP_FILES} ${Graphics_SRC_HEADERS})
source_group("src\\Graphics" FILES ${Graphics_SRC_CPP_FILES} ${Graphics_SRC_HEADERS})

target_sources(GEAR_CORE PRIVATE ${Graphics_Rendering_SRC_CPP_FILES} ${Graphics_Rendering_SRC_HEADERS})
source_group("src\\Graphics\\Rendering" FILES ${Graphics_Rendering_SRC_CPP_FILES} ${Graphics_Rendering_SRC_HEADERS})

target_sources(GEAR_CORE PRIVATE ${Graphics_Rendering_Passes_SRC_CPP_FILES} ${Graphics_Rendering_Passes_SRC_HEADERS})
source_group("src\\Graphics\\Rendering\\Passes" FILES ${Graphics_Rendering_Passes_SRC_CPP_FILES} ${Graphics_Rendering_Passes_SRC_HEADERS})

#Input
set(Input_SRC_HEADERS
	"src/Input/InputInterfaces.h"
	"src/Input/InputManager.h"
)
set(Input_SRC_CPP_FILES
	"src/Input/InputInterfaces.cpp"
	"src/Input/InputManager.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Input_SRC_CPP_FILES} ${Input_SRC_HEADERS})
source_group("src\\Input" FILES ${Input_SRC_CPP_FILES} ${Input_SRC_HEADERS})

#Objects
set(Objects_SRC_HEADERS
	"src/Objects/Camera.h"
	"src/Objects/Light.h"
	"src/Objects/Material.h"
	"src/Objects/Mesh.h"
	"src/Objects/Model.h"
	"src/Objects/ObjectInterface.h"
	"src/Objects/Probe.h"
	"src/Objects/Skybox.h"
	"src/Objects/Text.h"
	"src/Objects/Transform.h"
)
set(Objects_SRC_CPP_FILES
	"src/Objects/Camera.cpp"
	"src/Objects/Light.cpp"
	"src/Objects/Material.cpp"
	"src/Objects/Mesh.cpp"
	"src/Objects/Model.cpp"
	"src/Objects/ObjectInterface.cpp"
	"src/Objects/Probe.cpp"
	"src/Objects/Skybox.cpp"
	"src/Objects/Text.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Objects_SRC_CPP_FILES} ${Objects_SRC_HEADERS})
source_group("src\\Objects" FILES ${Objects_SRC_CPP_FILES} ${Objects_SRC_HEADERS})

#Scene
set(Scene_SRC_HEADERS
	"src/Scene/Components.h"
	"src/Scene/Entity.h"
	"src/Scene/NativeScript.h"
	"src/Scene/NativeScriptManager.h"
	"src/Scene/Scene.h"
)
set(Scene_SRC_CPP_FILES
	"src/Scene/Components.cpp"
	"src/Scene/NativeScriptManager.cpp"
	"src/Scene/Scene.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Scene_SRC_CPP_FILES} ${Scene_SRC_HEADERS})
source_group("src\\Scene" FILES ${Scene_SRC_CPP_FILES} ${Scene_SRC_HEADERS})

#UI
set(UI_SRC_HEADERS
	"src/UI/MenuBar.h"
	"src/UI/UIContext.h"
)
set(UI_Panels_SRC_HEADERS
	"src/UI/Panels/BasePanel.h"
	"src/UI/Panels/ContentBrowserPanel.h"
	"src/UI/Panels/ContentEditorPanel.h"
	"src/UI/Panels/MaterialPanel.h"
	"src/UI/Panels/OutputPanel.h"
	"src/UI/Panels/Panels.h"
	"src/UI/Panels/ProjectPanel.h"
	"src/UI/Panels/PropertiesPanel.h"
	"src/UI/Panels/RendererPropertiesPanel.h"
	"src/UI/Panels/SceneHierarchyPanel.h"
	"src/UI/Panels/ViewportPanel.h"
)
set(UI_ComponentUI_SRC_HEADERS
	"src/UI/ComponentUI/CameraComponentUI.h"
	"src/UI/ComponentUI/ComponentUI.h"
	"src/UI/ComponentUI/ComponentUIs.h"
	"src/UI/ComponentUI/LightComponentUI.h"
	"src/UI/ComponentUI/MaterialComponentUI.h"
	"src/UI/ComponentUI/ModelComponentUI.h"
	"src/UI/ComponentUI/NameComponentUI.h"
	"src/UI/ComponentUI/SkyboxComponentUI.h"
	"src/UI/ComponentUI/TransformComponentUI.h"
)
set(UI_SRC_CPP_FILES
	"src/UI/MenuBar.cpp"
	"src/UI/UIContext.cpp"
)
set(UI_Panels_SRC_CPP_FILES
	"src/UI/Panels/ContentBrowserPanel.cpp"
	"src/UI/Panels/ContentEditorPanel.cpp"
	"src/UI/Panels/MaterialPanel.cpp"
	"src/UI/Panels/OutputPanel.cpp"
	"src/UI/Panels/ProjectPanel.cpp"
	"src/UI/Panels/PropertiesPanel.cpp"
	"src/UI/Panels/RendererPropertiesPanel.cpp"
	"src/UI/Panels/SceneHierarchyPanel.cpp"
	"src/UI/Panels/ViewportPanel.cpp"
)
set(UI_ComponentUI_SRC_CPP_FILES
	"src/UI/ComponentUI/CameraComponentUI.cpp"
	"src/UI/ComponentUI/ComponentUI.cpp"
	"src/UI/ComponentUI/LightComponentUI.cpp"
	"src/UI/ComponentUI/MaterialComponentUI.cpp"
	"src/UI/ComponentUI/ModelComponentUI.cpp"
	"src/UI/ComponentUI/NameComponentUI.cpp"
	"src/UI/ComponentUI/SkyboxComponentUI.cpp"
	"src/UI/ComponentUI/TransformComponentUI.cpp"
)
target_sources(GEAR_CORE PRIVATE ${UI_SRC_CPP_FILES} ${UI_SRC_HEADERS})
source_group("src\\UI" FILES ${UI_SRC_CPP_FILES} ${UI_SRC_HEADERS})

target_sources(GEAR_CORE PRIVATE ${UI_Panels_SRC_CPP_FILES} ${UI_Panels_SRC_HEADERS})
source_group("src\\UI\\Panels" FILES ${UI_Panels_SRC_CPP_FILES} ${UI_Panels_SRC_HEADERS})

target_sources(GEAR_CORE PRIVATE ${UI_ComponentUI_SRC_CPP_FILES} ${UI_ComponentUI_SRC_HEADERS})
source_group("src\\UI\\ComponentUI" FILES ${UI_ComponentUI_SRC_CPP_FILES} ${UI_ComponentUI_SRC_HEADERS})

#ImGui
set(IMGUI_HEADERS
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/backends/imgui_impl_dx12.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/backends/imgui_impl_glfw.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/backends/imgui_impl_vulkan.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imconfig.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imgui.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imgui_internal.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imstb_rectpack.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imstb_textedit.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imstb_truetype.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/misc/cpp/imgui_stdlib.h"
)
set(IMGUI_CPP_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/backends/imgui_impl_dx12.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/backends/imgui_impl_glfw.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/backends/imgui_impl_vulkan.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imgui.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imgui_demo.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imgui_draw.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imgui_tables.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/imgui_widgets.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/misc/cpp/imgui_stdlib.cpp"
)
target_sources(GEAR_CORE PRIVATE ${IMGUI_CPP_FILES} ${IMGUI_HEADERS})
source_group("src\\ImGui" FILES ${IMGUI_CPP_FILES} ${IMGUI_HEADERS})

#Utils
set(Utils_SRC_HEADERS
	"src/Utils/FileUtils.h"
	"src/Utils/ModelLoader.h"
)
set(Utils_SRC_CPP_FILES
	"src/Utils/ModelLoader.cpp"
)
target_sources(GEAR_CORE PRIVATE ${Utils_SRC_CPP_FILES} ${Utils_SRC_HEADERS})
source_group("src\\Utils" FILES ${Utils_SRC_CPP_FILES} ${Utils_SRC_HEADERS})

#ASSIMP
target_include_directories(GEAR_CORE PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/ASSIMP/include/"
)
target_link_libraries(GEAR_CORE PRIVATE assimp)

#FREETYPE
target_include_directories(GEAR_CORE PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/FREETYPE/include/"
)
target_link_directories(GEAR_CORE PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/FREETYPE/release dll/win64"
)
target_link_libraries(GEAR_CORE PRIVATE
	freetype.lib
)
CopyToBinaryDirPostBuild(GEAR_CORE "${CMAKE_CURRENT_SOURCE_DIR}/../External/FREETYPE/release dll/win64/freetype.dll" "")

#GLFW
target_include_directories(GEAR_CORE PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/GLFW/include/"
)
target_link_directories(GEAR_CORE PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/GLFW/lib-vc2022/"
)
target_link_libraries(GEAR_CORE PRIVATE
	glfw3.lib
)

target_include_directories(GEAR_CORE PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/imgui/"
)

#MIRU - D3D12 and Vulkan
target_include_directories(GEAR_CORE PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/MIRU/MIRU_CORE/src/"
)
if (WIN32)
	target_include_directories(GEAR_CORE PRIVATE 
			"${CMAKE_BINARY_DIR}/packages/Microsoft.Direct3D.D3D12.1.610.5/"
			"${CMAKE_CURRENT_SOURCE_DIR}/../External/MIRU/External/D3D12MemoryAllocator/include/"
	)
	target_link_libraries(GEAR_CORE PRIVATE d3d12.lib dxgi.lib)
endif()
find_package(Vulkan)
if (Vulkan_FOUND)
	set(Vulkan_SDK_DIR "${Vulkan_INCLUDE_DIRS}/../")
	target_include_directories(GEAR_CORE PRIVATE ${Vulkan_INCLUDE_DIRS})
	target_link_directories(GEAR_CORE PRIVATE "${Vulkan_SDK_DIR}/Lib")
	target_link_libraries(GEAR_CORE PRIVATE vulkan-1.lib)
endif()

#XAudio2
target_link_libraries(GEAR_CORE PRIVATE
	xaudio2.lib
)

#XInput
target_link_libraries(GEAR_CORE PRIVATE
	xinput.lib
)

#Includes
target_include_directories(GEAR_CORE PRIVATE 
	"${CMAKE_CURRENT_SOURCE_DIR}/src/"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/"
	"${CMAKE_CURRENT_SOURCE_DIR}/../External/MIRU/External/"
)

#Defines
target_compile_definitions(GEAR_CORE PRIVATE GEAR_BUILD_DLL)

#Links
target_link_libraries(GEAR_CORE PRIVATE 
	MIRU_CORE
	ARC
)

#Shaders
set(GEAR_CORE_RENDER_PIPELINE_FILES
	"res/pipelines/BloomDownsample.grpf"
	"res/pipelines/BloomPrefilter.grpf"
	"res/pipelines/BloomUpsample.grpf"
	"res/pipelines/Cube.grpf"
	"res/pipelines/DebugCoordinateAxes.grpf"
	"res/pipelines/DebugCopy.grpf"
	"res/pipelines/DebugShowDepth.grpf"
	"res/pipelines/DebugShowDepthCubemap.grpf"
	"res/pipelines/DiffuseIrradiance.grpf"
	"res/pipelines/EquirectangularToCube.grpf"
	"res/pipelines/HDR.grpf"
	"res/pipelines/Mipmap.grpf"
	"res/pipelines/MipmapArray.grpf"
	"res/pipelines/PBROpaque.grpf"
	"res/pipelines/Shadow.grpf"
	"res/pipelines/SpecularBRDF_LUT.grpf"
	"res/pipelines/SpecularIrradiance.grpf"
	"res/pipelines/Text.grpf"
)

set(GEAR_CORE_SHADER_PBR_FILES
	"res/shaders/PBR/DiffuseIrradiance.hlsl"
	"res/shaders/PBR/PBRFunctions.h"
	"res/shaders/PBR/PBROpaque.hlsl"
	"res/shaders/PBR/SpecularBRDF_LUT.hlsl"
	"res/shaders/PBR/SpecularIrradiance.hlsl"
)
set(GEAR_CORE_SHADER_PostProcess_FILES
	"res/shaders/PostProcess/Bloom.hlsl"
	"res/shaders/PostProcess/HDR.hlsl"
)
set(GEAR_CORE_SHADER_FILES
	"res/shaders/Cube.hlsl"
	"res/shaders/CubeFunctions.h"
	"res/shaders/Debug.hlsl"
	"res/shaders/Depth.h"
	"res/shaders/EquirectangularToCube.hlsl"
	"res/shaders/Mipmap.hlsl"
	"res/shaders/RayTracing.hlsl"
	"res/shaders/Shadow.hlsl"
	"res/shaders/Text.hlsl"

	"res/shaders/shadertoolsconfig.json"
)

set(GEAR_CORE_SHADER_PBR_JSON_FILES
	"res/shaders/PBR/DiffuseIrradiance.json"
	"res/shaders/PBR/PBROpaque.json"
	"res/shaders/PBR/SpecularBRDF_LUT.json"
	"res/shaders/PBR/SpecularIrradiance.json"
)

set(GEAR_CORE_SHADER_PostProcess_JSON_FILES
	"res/shaders/PostProcess/Bloom.json"
	"res/shaders/PostProcess/HDR.json"
)

set(GEAR_CORE_SHADER_JSON_FILES
	"res/shaders/Cube.json"
	"res/shaders/Debug.json"
	"res/shaders/EquirectangularToCube.json"
	"res/shaders/Mipmap.json"
	"res/shaders/Shadow.json"
	"res/shaders/Text.json"
)

target_sources(GEAR_CORE PRIVATE ${GEAR_CORE_RENDER_PIPELINE_FILES})
source_group("res\\pipelines" FILES ${GEAR_CORE_RENDER_PIPELINE_FILES})

target_sources(GEAR_CORE PRIVATE 
	${GEAR_CORE_SHADER_PBR_FILES} 
	${GEAR_CORE_SHADER_PostProcess_FILES} 
	${GEAR_CORE_SHADER_FILES} 
	${GEAR_CORE_SHADER_PBR_JSON_FILES} 
	${GEAR_CORE_SHADER_PostProcess_JSON_FILES} 
	${GEAR_CORE_SHADER_JSON_FILES}
)
source_group("res\\shaders\\PBR" FILES 
	${GEAR_CORE_SHADER_PBR_FILES} 
	${GEAR_CORE_SHADER_PBR_JSON_FILES}
)
source_group("res\\shaders\\PostProcess" FILES 
	${GEAR_CORE_SHADER_PostProcess_FILES} 
	${GEAR_CORE_SHADER_PostProcess_JSON_FILES}
)
source_group("res\\shaders" FILES 
	${GEAR_CORE_SHADER_FILES} 
	${GEAR_CORE_SHADER_JSON_FILES}
)

set_property(SOURCE 
	${GEAR_CORE_SHADER_PBR_FILES} 
	${GEAR_CORE_SHADER_PostProcess_FILES} 
	${GEAR_CORE_SHADER_FILES} 
	PROPERTY VS_SETTINGS "ExcludedFromBuild=true"
)

target_compile_definitions(GEAR_CORE PRIVATE "BUILD_DIR=\"${CMAKE_BINARY_DIR}\"")
target_compile_definitions(GEAR_CORE PRIVATE "SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"")

set(HLSL_JSON_FILES 
	${GEAR_CORE_SHADER_PBR_JSON_FILES} 
	${GEAR_CORE_SHADER_PostProcess_JSON_FILES} 
	${GEAR_CORE_SHADER_JSON_FILES}
)

add_dependencies(GEAR_CORE MIRU_SHADER_COMPILER)
add_custom_command(TARGET GEAR_CORE POST_BUILD
	COMMAND MIRU_SHADER_COMPILER -raf:${HLSL_JSON_FILES} 
		-rafd:$BUILD_DIR=\"${CMAKE_BINARY_DIR}\"
		-rafd:$SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

#foreach(HLSL_JSON_FILE ${HLSL_JSON_FILES})
#	CopyToBuildDirPostBuild(GEAR_CORE "${CMAKE_CURRENT_SOURCE_DIR}/${HLSL_JSON_FILE}" "shaderbin")
#endforeach()