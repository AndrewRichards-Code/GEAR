#pragma once
#include "ObjectInterface.h"
#include "Core/AssetFile.h"
#include "gear_core_common.h"
#include "Graphics/RenderPipeline.h"
#include "Graphics/Texture.h"
#include "Graphics/Uniformbuffer.h"

namespace gear 
{
namespace objects 
{
	class GEAR_API Material : public ObjectComponentInterface
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
	
		struct CreateInfo : public ObjectComponentInterface::CreateInfo
		{
			std::map<TextureType, Ref<graphics::Texture>>	pbrTextures;
			graphics::UniformBufferStructures::PBRConstants	pbrConstants;
			std::string										filepath; //Optional
		};

	private:
		static Ref<graphics::Texture> s_WhiteTexture;
		static Ref<graphics::Texture> s_BlueNormalTexture;
		static Ref<graphics::Texture> s_BlackTexture;
		static std::map<std::string, Ref<Material>> s_LoadedMaterials;
		
		typedef graphics::UniformBufferStructures::PBRConstants PBRConstantsUB;
		Ref<graphics::Uniformbuffer<PBRConstantsUB>> m_UB;
	
		struct Properties
		{
			std::string name;
			int				twoSided;
			int				shadingModel;
			int				wireframe;
			int				blendFunc;
			float			opacity;
			float			shininess;
			float			reflectivity;
			float			shininessStrength;
			float			refractiveIndex;
			mars::float4	colourDiffuse;
			mars::float4	colourAmbient;
			mars::float4	colourSpecular;
			mars::float4	colourEmissive;
			mars::float4	colourTransparent;
			mars::float4	colourReflective;
		} m_Properties;
	
	public:
		CreateInfo m_CI;
	
	public:
		Material(CreateInfo* pCreateInfo);
		~Material();
	
		//Update material based current Material::CreateInfo m_CI.
		void Update() override;

	protected:
		bool CreateInfoHasChanged(const ObjectComponentInterface::CreateInfo* pCreateInfo) override;

	public:
		void SetDefaultPBRTextures();
		void AddProperties(const Properties& properties);
		
	private:
		void InitialiseUB();
		void CreateDefaultColourTextures();

	public:
		void LoadFromAssetFile(const core::AssetFile& inAssetFile);
		void SaveToAssetFile(core::AssetFile& outAssetFile);
	
		inline std::map<TextureType, Ref<graphics::Texture>>& GetTextures() { return m_CI.pbrTextures; }
		inline const std::map<TextureType, Ref<graphics::Texture>>& GetTextures() const { return m_CI.pbrTextures; }
		inline const Ref<graphics::Uniformbuffer<PBRConstantsUB>>& GetUB() const { return m_UB; }

		inline std::string GetDebugName() const { return "GEAR_CORE_Material: " + m_CI.debugName; }
	
		inline static void AddMaterial(std::string name, const Ref<Material>& material) { s_LoadedMaterials.insert({ name, material }); }
		inline static Ref<Material> FindMaterial(const std::string& name) 
		{ 
			std::map<std::string, Ref<Material>>::iterator it = s_LoadedMaterials.find(name);
			if (it != s_LoadedMaterials.end())
				return it->second;
			else
				return nullptr;
		}
	};
}
}