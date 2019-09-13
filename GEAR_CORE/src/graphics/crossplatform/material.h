#pragma once

#include "gear_common.h"
#include "ARMLib.h"
#include "graphics/opengl/shader/shader.h"
#include "graphics/opengl/texture.h"

namespace GEAR {
namespace GRAPHICS {
namespace CROSSPLATFORM {

#ifdef GEAR_OPENGL
class Material
{
public:
	enum class TextureType : int
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
	static bool s_InitialiseUBO;
	struct PBRInfoUBO
	{
		ARM::Vec4 m_Fersnel;
		ARM::Vec4 m_Albedo;
		float m_Metallic;
		float m_Roughness;
		float m_AmbientOcclusion;
		float m_Pad;
	}m_PBRInfoUBO;

	struct Properties
	{
		std::string m_Name;
		int m_TwoSided;
		int m_ShadingModel;
		int m_Wireframe;
		int m_BlendFunc;
		float m_Opacity;
		float m_Shininess;
		float m_Reflectivity;
		float m_ShininessStrength;
		float m_RefractiveIndex;
		ARM::Vec4 m_ColourDiffuse;
		ARM::Vec4 m_ColourAmbient;
		ARM::Vec4 m_ColourSpecular;
		ARM::Vec4 m_ColourEmissive;
		ARM::Vec4 m_ColourTransparent;
		ARM::Vec4 m_ColourReflective;
	}m_Properties;

	const OPENGL::Shader& m_Shader;
	std::map<const OPENGL::Texture*, TextureType> m_Textures;

public:
	Material(const OPENGL::Shader& shader);
	~Material();

	void AddTexture(const OPENGL::Texture& texture, TextureType type = TextureType::GEAR_TEXTURE_UNKNOWN);
	void ChangeTextureType(const OPENGL::Texture& texture, TextureType type);
	void AddProperties(std::string name, int twoSided, int shadingModel, int wireframe, int blendFunc, float opacity, float shininess, float reflectivity, float shininessStrength, float refractiveIndex, const ARM::Vec4& colourDiffuse, const ARM::Vec4& colourAmbient, const ARM::Vec4& colourSpecular, const ARM::Vec4& colourEmissive, const ARM::Vec4& colourTransparent, const ARM::Vec4& colourReflective);
	void SetPBRParameters(const ARM::Vec4& fersnel = ARM::Vec4(0, 0, 0, 0), const ARM::Vec4& albedo = ARM::Vec4(0, 0, 0, 0), float metallic = 0.0f, float roughness = 0.0f, float ambientOcclusion = 0.0f);
	void BindPBRTextures();
	void UnbindPBRTextures();

	inline const OPENGL::Shader& GetShader() const { return m_Shader; }
	inline const std::map<const OPENGL::Texture*, TextureType>& GetTextures() const { return m_Textures; }

private:
	void InitialiseUBO();
	void SetAllToZero();
};
#endif

}
}
}