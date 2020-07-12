#pragma once

#include "gear_core_common.h"
#include "graphics/miru/pipeline.h"
#include "graphics/miru/texture.h"
#include "graphics/miru/buffer/uniformbuffer.h"

namespace gear {
namespace objects {

class Material
{
public:
	struct CreateInfo
	{
		void*							device;
		gear::Ref<graphics::Pipeline>	pipeline;
	};

	enum class TextureType : uint32_t
	{
		GEAR_TEXTURE_UNKNOWN = 0,		//Pre-Initialised value
		GEAR_TEXTURE_DIFFUSE,			//For diffuse
		GEAR_TEXTURE_SPECULAR,			//For specular
		GEAR_TEXTURE_AMBIENT,			//For ambient
		GEAR_TEXTURE_EMISSIVE,			//For emissive
		GEAR_TEXTURE_NORMAL,			//For normal maps
		GEAR_TEXTURE_ALBEDO,			//For PBR - Diffuse lighting
		GEAR_TEXTURE_METALLIC,			//For PBR - Specifies metallic (sub-surface scattering)
		GEAR_TEXTURE_ROUGHNESS,			//For PBR - Num of Micro-facets (1.0 - Smootheness)
		GEAR_TEXTURE_SMOOTHNESS,		//For PBR - Num of Micro-facets (1.0 - Roughness)
		GEAR_TEXTURE_AMBIENT_OCCLUSION,	//For PBR - Micro-facets shadowing
		GEAR_TEXTURE_OPACITY,			//For opacity
		GEAR_TEXTURE_REFLECTION,		//For reflections
		GEAR_TEXTURE_DUDV,				//For changing UV values
		GEAR_TEXTURE_HEIGHT,			//For height maps
		GEAR_TEXTURE_DISPLACEMENT,		//For displacement of vertices
	};

private:
	struct PBRInfoUBO
	{
		mars::Vec4	fersnel;
		mars::Vec4	albedo;
		float		metallic;
		float		roughness;
		float		ambientOcclusion;
		float		pad;
	}m_PBRInfoUBO;

	struct Properties
	{
		std::string name;
		int			twoSided;
		int			shadingModel;
		int			wireframe;
		int			blendFunc;
		float		opacity;
		float		shininess;
		float		reflectivity;
		float		shininessStrength;
		float		refractiveIndex;
		mars::Vec4	colourDiffuse;
		mars::Vec4	colourAmbient;
		mars::Vec4	colourSpecular;
		mars::Vec4	colourEmissive;
		mars::Vec4	colourTransparent;
		mars::Vec4	colourReflective;
	}m_Properties;

	CreateInfo m_CI;
	std::map<gear::Ref<graphics::Texture>, TextureType> m_Textures;
	gear::Ref<graphics::UniformBuffer> m_UB;

public:
	Material(CreateInfo* pCreateInfo);
	~Material();

	void AddTexture(gear::Ref<graphics::Texture> texture, TextureType type = TextureType::GEAR_TEXTURE_UNKNOWN);
	void ChangeTextureType(gear::Ref<graphics::Texture> texture, TextureType type);
	void AddProperties(const Properties& properties);
	void SetPBRParameters(const mars::Vec4& fersnel = mars::Vec4(0, 0, 0, 0), const mars::Vec4& albedo = mars::Vec4(0, 0, 0, 0), float metallic = 0.0f, float roughness = 0.0f, float ambientOcclusion = 0.0f);

	inline const gear::Ref<graphics::Pipeline>& GetPipeline() const { return m_CI.pipeline; }
	inline const std::map<gear::Ref<graphics::Texture>, TextureType>& GetTextures() const { return m_Textures; }

private:
	void InitialiseUB();
};
}
}