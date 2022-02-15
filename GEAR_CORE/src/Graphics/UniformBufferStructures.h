#ifndef UNIFORM_BUFFER_STRUCTURES_H
#define UNIFORM_BUFFER_STRUCTURES_H

#include "../Core/TypesCppHlsl.h"

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

			struct BloomInfo
			{
				GEAR_FLOAT		threshold;
				GEAR_FLOAT		upsampleScale;
			};

			//Per model - Set 1

			struct Model
			{
				GEAR_FLOAT4X4	modl;
				GEAR_FLOAT2		texCoordScale0;
				GEAR_FLOAT2		texCoordScale1;
			};

			//Per material - Set 2

			struct HDRInfo
			{
				GEAR_FLOAT		exposure;
				GEAR_UINT		gammaSpace; //0 = Linear, 1 = sRGB, 2 = Rec709, 3 = Rec2020. See ColourSpace. enum?
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
			};
				
			#ifdef __cplusplus
			static PBRConstants DefaultPBRConstants()
			{
				PBRConstants pbrConstants;
				pbrConstants.fresnel = mars::float4(1, 1, 1, 1);
				pbrConstants.albedo = mars::float4(1, 1, 1, 1);
				pbrConstants.metallic = 1.0f;
				pbrConstants.roughness = 1.0f;
				pbrConstants.ambientOcclusion = 1.0f;
				pbrConstants.pad = 0.0f;
				pbrConstants.emissive = mars::float4(0, 0, 0, 1);
				return pbrConstants;
			}
			#endif	

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
			{ "TEXTCAMERA",		SetUpdateType::PER_VIEW		},
			{ "LIGHTS",			SetUpdateType::PER_VIEW		},
			{ "HDRINFO",		SetUpdateType::PER_VIEW		},
			{ "MODEL",			SetUpdateType::PER_MODEL	},
			{ "PBRCONSTANTS",	SetUpdateType::PER_MATERIAL }
		};
	}
}
#endif

#endif // UNIFORM_BUFFER_STRUCTURES_H
