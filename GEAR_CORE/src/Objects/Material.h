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
			EMISSIVE,			//For PBR - Any light that the surface emits
		};
	
		struct CreateInfo
		{
			std::string											debugName;
			void*												device;
			std::map<TextureType, gear::Ref<graphics::Texture>> pbrTextures;
			graphics::UniformBufferStructures::PBRConstants		pbrConstants;
		};
	
	private:
		static gear::Ref<graphics::Texture> s_WhiteTexture;
		static gear::Ref<graphics::Texture> s_BlueNormalTexture;
		static gear::Ref<graphics::Texture> s_BlackTexture;
		static std::map<std::string, gear::Ref<Material>> s_LoadedMaterials;
		
		typedef graphics::UniformBufferStructures::PBRConstants PBRConstantsUB;
		gear::Ref<graphics::Uniformbuffer<PBRConstantsUB>> m_UB;
	
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
	
		//Update material based current Material::CreateInfo m_CI.
		void Update();

		void AddProperties(const Properties& properties);
		
		inline std::map<TextureType, gear::Ref<graphics::Texture>>& GetTextures() { return m_CI.pbrTextures; }
		inline const std::map<TextureType, gear::Ref<graphics::Texture>>& GetTextures() const { return m_CI.pbrTextures; }
		inline const gear::Ref<graphics::Uniformbuffer<PBRConstantsUB>>& GetUB() const { return m_UB; }

		inline std::string GetDebugName() const { return "GEAR_CORE_Material: " + m_CI.debugName; }
	
		inline static void AddMaterial(std::string name, const gear::Ref<Material>& material) { s_LoadedMaterials.insert({ name, material }); }
		inline static gear::Ref<Material> FindMaterial(const std::string& name) 
		{ 
			std::map<std::string, gear::Ref<Material>>::iterator it = s_LoadedMaterials.find(name);
			if (it != s_LoadedMaterials.end())
				return it->second;
			else
				return nullptr;
		}
	
	private:
		void InitialiseUB();
		void CreateDefaultColourTextures();
	};
}
}