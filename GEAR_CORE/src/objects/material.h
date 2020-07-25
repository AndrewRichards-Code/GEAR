#pragma once

#include "gear_core_common.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Texture.h"
#include "Graphics/Uniformbuffer.h"

namespace gear {
namespace objects {

class Material
{
public:
	struct CreateInfo
	{
		const char*							debugName;
		void*								device;
		gear::Ref<graphics::RenderPipeline>	pRenderPipeline;
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
	struct PBRInfoUB
	{
		mars::Vec4	fersnel;
		mars::Vec4	albedo;
		float		metallic;
		float		roughness;
		float		ambientOcclusion;
		float		pad;
	};
	gear::Ref<graphics::Uniformbuffer<PBRInfoUB>> m_UB;

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
	} m_Properties;

	std::map<gear::Ref<graphics::Texture>, TextureType> m_Textures;

	CreateInfo m_CI;

public:
	Material(CreateInfo* pCreateInfo);
	~Material();

	void AddTexture(gear::Ref<graphics::Texture> texture, TextureType type = TextureType::GEAR_TEXTURE_UNKNOWN);
	void ChangeTextureType(gear::Ref<graphics::Texture> texture, TextureType type);
	void AddProperties(const Properties& properties);
	void SetPBRParameters(const mars::Vec4& fersnel = mars::Vec4(0, 0, 0, 0), const mars::Vec4& albedo = mars::Vec4(0, 0, 0, 0), float metallic = 0.0f, float roughness = 0.0f, float ambientOcclusion = 0.0f);

	inline const gear::Ref<graphics::RenderPipeline>& GetRenderPipeline() const { return m_CI.pRenderPipeline; }
	inline const std::map<gear::Ref<graphics::Texture>, TextureType>& GetTextures() const { return m_Textures; }

private:
	void InitialiseUB();
};
}
}