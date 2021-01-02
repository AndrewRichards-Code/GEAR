//CPP
#ifdef __cplusplus

#pragma once
#include "mars.h"

#define GEAR_FLOAT		float
#define GEAR_FLOAT2		mars::Vec2
#define GEAR_FLOAT3		mars::Vec3
#define GEAR_FLOAT4		mars::Vec4
#define GEAR_FLOAT2X2	mars::Mat2
#define GEAR_FLOAT3X3	mars::Mat3
#define GEAR_FLOAT4X4	mars::Mat4

#define GEAR_DOUBLE		double
#define GEAR_DOUBLE2	mars::Double2
#define GEAR_DOUBLE3	mars::Double3
#define GEAR_DOUBLE4	mars::Double4
#define GEAR_DOUBLE2X2	mars::DMat2
#define GEAR_DOUBLE3X3	mars::DMat3
#define GEAR_DOUBLE4X4	mars::DMat4

#define GEAR_INT		int32_t
#define GEAR_INT2		mars::Int2
#define GEAR_INT3		mars::Int3
#define GEAR_INT4		mars::Int4
#define GEAR_INT2X2		mars::IMat2
#define GEAR_INT3X3		mars::IMat3
#define GEAR_INT4X4		mars::IMat4

#define GEAR_UINT		uint32_t
#define GEAR_UINT2		mars::Uint2
#define GEAR_UINT3		mars::Uint3
#define GEAR_UINT4		mars::Uint4
#define GEAR_UINT2X2	mars::UIMat2
#define GEAR_UINT3X3	mars::UIMat3
#define GEAR_UINT4X4	mars::UIMat4

//HLSL
#else 

#define GEAR_FLOAT		float
#define GEAR_FLOAT2		float2
#define GEAR_FLOAT3		float3
#define GEAR_FLOAT4		float4
#define GEAR_FLOAT2X2	float2x2
#define GEAR_FLOAT3X3	float3x3
#define GEAR_FLOAT4X4	float4x4

#define GEAR_DOUBLE		double
#define GEAR_DOUBLE2	double2
#define GEAR_DOUBLE3	double3
#define GEAR_DOUBLE4	double4
#define GEAR_DOUBLE2X2	double2x2
#define GEAR_DOUBLE3X3	double3x3
#define GEAR_DOUBLE4X4	double4x4

#define GEAR_INT		int
#define GEAR_INT2		int2
#define GEAR_INT3		int3
#define GEAR_INT4		int4
#define GEAR_INT2X2		int2x2
#define GEAR_INT3X3		int3x3
#define GEAR_INT4X4		int4x4

#define GEAR_UINT		uint
#define GEAR_UINT2		uint2
#define GEAR_UINT3		uint3
#define GEAR_UINT4		uint4
#define GEAR_UINT2X2	uint2x2
#define GEAR_UINT3X3	uint3x3
#define GEAR_UINT4X4	uint4x4

#endif

#ifdef __cplusplus
namespace gear
{
	namespace graphics
	{
		struct UniformBufferStructures
		{
#endif
			//Per view - Set 0

			struct Camera
			{
				GEAR_FLOAT4X4	proj;
				GEAR_FLOAT4X4	view;
				GEAR_FLOAT4		cameraPosition;
			};

			struct Light
			{
				GEAR_FLOAT4		colour;
				GEAR_FLOAT4		position;
				GEAR_FLOAT4		direction;
				GEAR_FLOAT4		valid;
			};

			struct Lights
			{
				Light			lights[8];
			};

			struct SpecularIrradianceInfo
			{
				GEAR_FLOAT		roughness;
			};

			//Per model - Set 1

			struct Model
			{
				GEAR_FLOAT4X4	modl;
				GEAR_FLOAT2		texCoordScale0;
				GEAR_FLOAT2		texCoordScale1;
			};

			//Per material - Set 2

			struct SkyboxInfo
			{
				GEAR_FLOAT		exposure;
				GEAR_FLOAT		gamma;
			};

			struct PBRConstants
			{
				GEAR_FLOAT4		fresnel;
				GEAR_FLOAT4		albedo;
				GEAR_FLOAT		metallic;
				GEAR_FLOAT		roughness;
				GEAR_FLOAT		ambientOcclusion;
				GEAR_FLOAT		pad;
				GEAR_FLOAT4		emissive;
				
			#ifdef __cplusplus
				PBRConstants() :
					fresnel(1, 1, 1, 1),
					albedo(1, 1, 1, 1),
					metallic(1),
					roughness(1),
					ambientOcclusion(1),
					pad(0),
					emissive(1, 1, 1, 1) {}
			#endif	

			};
#ifdef __cplusplus
		};

		enum class SetUpdateType : uint32_t
		{
			PER_VIEW,
			PER_MODEL,
			PER_MATERIAL,
			UNKNOWN
		};

		static std::map<std::string, SetUpdateType> SetUpdateTypeMap =
		{
			{ "CAMERA",			SetUpdateType::PER_VIEW		},
			{ "FONTCAMERA",		SetUpdateType::PER_VIEW		},
			{ "LIGHTS",			SetUpdateType::PER_VIEW		},
			{ "SKYBOXINFO",		SetUpdateType::PER_MATERIAL	},
			{ "MODEL",			SetUpdateType::PER_MODEL	},
			{ "PBRCONSTANTS",	SetUpdateType::PER_MATERIAL }
		};
	}
}
#endif