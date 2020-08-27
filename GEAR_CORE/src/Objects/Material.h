#pragma once

#include "gear_core_common.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Texture.h"
#include "Graphics/Uniformbuffer.h"

namespace gear 
{
namespace objects 
{
	class Material
	{
	public:
		enum class TextureType : uint32_t
		{
			UNKNOWN = 0,		//Pre-Initialised value
			NORMAL,				//For normal maps
			ALBEDO,				//For PBR - Base Albedo colour
			METALLIC,			//For PBR - Specifies metallic (sub-surface scattering)
			ROUGHNESS,			//For PBR - Num of Micro-facets (1.0 - Smootheness)
			AMBIENT_OCCLUSION,	//For PBR - Micro-facets shadowing
			EMISSIVE,			//For PBR - Anu light that the surface emits
		};
	
		struct PBRConstants
		{
			mars::Vec4	fersnel				= mars::Vec4(1, 1, 1, 1);
			mars::Vec4	albedo				= mars::Vec4(1, 1, 1, 1);
			float		metallic			= 1.0f;
			float		roughness			= 1.0f;
			float		ambientOcclusion	= 1.0f;
			float		pad					= 0.0f; //Value ignored.
			mars::Vec4	emissive			= mars::Vec4(1, 1, 1, 1);
		};
	
		struct CreateInfo
		{
			std::string											debugName;
			void*												device;
			std::map<TextureType, gear::Ref<graphics::Texture>> pbrTextures;
			PBRConstants										pbrConstants;
		};
	
	private:
		gear::Ref<graphics::Texture> s_WhiteTexture;
		gear::Ref<graphics::Texture> s_BlueTexture;
		gear::Ref<graphics::Texture> s_BlackTexture;
		static std::map<std::string, gear::Ref<Material>> s_LoadedMaterials;
		
		gear::Ref<graphics::Uniformbuffer<PBRConstants>> m_UB;
	
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
	
	public:
		CreateInfo m_CI;
	
	public:
		Material(CreateInfo* pCreateInfo);
		~Material();
	
		//Update material based current CreateInfo.
		void Update();
		void AddProperties(const Properties& properties);
		
		inline const std::map<TextureType, gear::Ref<graphics::Texture>>& GetTextures() const { return m_CI.pbrTextures; }
		inline const gear::Ref<graphics::Uniformbuffer<PBRConstants>>& GetUB() const { return m_UB; }
	
		/*inline static void AddMaterial(std::string name, const gear::Ref<Material>& material) { s_LoadedMaterials.insert({ name, material }); }
		inline static gear::Ref<Material> FindMaterial(const std::string& name) 
		{ 
			std::map<std::string, gear::Ref<Material>>::iterator it = s_LoadedMaterials.find(name);
			if (it != s_LoadedMaterials.end())
				return it->second;
			else
				return nullptr;
		}*/
	
	private:
		void InitialiseUB();
		void CreateDefaultColourTextures();
	};
}
}