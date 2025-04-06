cmake_minimum_required(VERSION 3.27)

set(TARGET_NAME GEAR_CORE_Resources)
message(STATUS "${TARGET_NAME}")

add_custom_target(${TARGET_NAME})

#Pipeline
set(GEAR_CORE_RENDER_PIPELINE_FILES
	"Pipelines/BloomDownsample.grpf"
	"Pipelines/BloomPrefilter.grpf"
	"Pipelines/BloomUpsample.grpf"
	"Pipelines/Cube.grpf"
	"Pipelines/DebugBoxes.grpf"
	"Pipelines/DebugCoordinateAxes.grpf"
	"Pipelines/DebugCopy.grpf"
	"Pipelines/DebugShowDepth.grpf"
	"Pipelines/DebugShowDepthCubemap.grpf"
	"Pipelines/DiffuseIrradiance.grpf"
	"Pipelines/EquirectangularToCube.grpf"
	"Pipelines/HDR.grpf"
	"Pipelines/Mipmap.grpf"
	"Pipelines/MipmapArray.grpf"
	"Pipelines/PBROpaque.grpf"
	"Pipelines/ShadowCascades.grpf"
	"Pipelines/ShadowSingle.grpf"
	"Pipelines/SpecularBRDF_LUT.grpf"
	"Pipelines/SpecularIrradiance.grpf"
	"Pipelines/Text.grpf"
)

set(GEAR_CORE_SHADER_PBR_FILES
	"Shaders/PBR/DiffuseIrradiance.hlsl"
	"Shaders/PBR/PBRFunctions.h"
	"Shaders/PBR/PBROpaque.hlsl"
	"Shaders/PBR/SpecularBRDF_LUT.hlsl"
	"Shaders/PBR/SpecularIrradiance.hlsl"
)
set(GEAR_CORE_SHADER_PostProcess_FILES
	"Shaders/PostProcess/Bloom.hlsl"
	"Shaders/PostProcess/HDR.hlsl"
)
set(GEAR_CORE_SHADER_FILES
	"Shaders/Cube.hlsl"
	"Shaders/CubeFunctions.h"
	"Shaders/Debug.hlsl"
	"Shaders/Depth.h"
	"Shaders/EquirectangularToCube.hlsl"
	"Shaders/Mipmap.hlsl"
	"Shaders/RayTracing.hlsl"
	"Shaders/Shadow.hlsl"
	"Shaders/Text.hlsl"

	"Shaders/shadertoolsconfig.json"
)

#Pipeline
set(GEAR_CORE_SHADER_PBR_JSON_FILES
	"Shaders/PBR/DiffuseIrradiance.json"
	"Shaders/PBR/PBROpaque.json"
	"Shaders/PBR/SpecularBRDF_LUT.json"
	"Shaders/PBR/SpecularIrradiance.json"
)

set(GEAR_CORE_SHADER_PostProcess_JSON_FILES
	"Shaders/PostProcess/Bloom.json"
	"Shaders/PostProcess/HDR.json"
)

set(GEAR_CORE_SHADER_JSON_FILES
	"Shaders/Cube.json"
	"Shaders/Debug.json"
	"Shaders/EquirectangularToCube.json"
	"Shaders/Mipmap.json"
	"Shaders/Shadow.json"
	"Shaders/Text.json"
)

#Add Pipelines to project
target_sources(${TARGET_NAME} PRIVATE ${GEAR_CORE_RENDER_PIPELINE_FILES})
source_group("Pipelines" FILES ${GEAR_CORE_RENDER_PIPELINE_FILES})

#Add Shaders to project
target_sources(${TARGET_NAME} PRIVATE 
	${GEAR_CORE_SHADER_PBR_FILES} 
	${GEAR_CORE_SHADER_PostProcess_FILES} 
	${GEAR_CORE_SHADER_FILES} 
	${GEAR_CORE_SHADER_PBR_JSON_FILES} 
	${GEAR_CORE_SHADER_PostProcess_JSON_FILES} 
	${GEAR_CORE_SHADER_JSON_FILES}
)
source_group("Shaders\\PBR" FILES 
	${GEAR_CORE_SHADER_PBR_FILES} 
	${GEAR_CORE_SHADER_PBR_JSON_FILES}
)
source_group("Shaders\\PostProcess" FILES 
	${GEAR_CORE_SHADER_PostProcess_FILES} 
	${GEAR_CORE_SHADER_PostProcess_JSON_FILES}
)
source_group("Shaders" FILES 
	${GEAR_CORE_SHADER_FILES} 
	${GEAR_CORE_SHADER_JSON_FILES}
)

#Exclude shaders from build
set_property(SOURCE 
	${GEAR_CORE_SHADER_PBR_FILES} 
	${GEAR_CORE_SHADER_PostProcess_FILES} 
	${GEAR_CORE_SHADER_FILES} 
	PROPERTY VS_SETTINGS "ExcludedFromBuild=true"
)

set(HLSL_JSON_FILES 
	${GEAR_CORE_SHADER_PBR_JSON_FILES} 
	${GEAR_CORE_SHADER_PostProcess_JSON_FILES} 
	${GEAR_CORE_SHADER_JSON_FILES}
)

#Post-build compile command
add_dependencies(${TARGET_NAME} MIRU_SHADER_COMPILER)
add_custom_command(TARGET ${TARGET_NAME} PRE_BUILD
	COMMAND MIRU_SHADER_COMPILER -raf:${HLSL_JSON_FILES} 
		-rafd:$BUILD_DIR=\"${CMAKE_BINARY_DIR}\"
		-rafd:$SOURCE_DIR=\"${CMAKE_SOURCE_DIR}\"
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
add_dependencies(GEAR_CORE_Graphics ${TARGET_NAME})

#foreach(HLSL_JSON_FILE ${HLSL_JSON_FILES})
#	CopyToBuildDirPostBuild(GEAR_CORE "${CMAKE_CURRENT_SOURCE_DIR}/${HLSL_JSON_FILE}" "shaderbin")
#endforeach()