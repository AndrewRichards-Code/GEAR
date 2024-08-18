#pragma once
#include "gear_core_common.h"
#include "Objects/ObjectInterface.h"
#include "Asset/AssetFile.h"
#include "Graphics/Texture.h"
#include "Graphics/Uniformbuffer.h"

namespace gear 
{
	namespace objects
	{
		class GEAR_API Material : public ObjectComponentInterface, public asset::Asset
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
			};

		private:
			static Ref<graphics::Texture> s_WhiteTexture;
			static Ref<graphics::Texture> s_BlueNormalTexture;
			static Ref<graphics::Texture> s_BlackTexture;

			typedef graphics::UniformBufferStructures::PBRConstants PBRConstantsUB;
			Ref<graphics::Uniformbuffer<PBRConstantsUB>> m_UB;

		public:
			CreateInfo m_CI;

			Material(CreateInfo* pCreateInfo);
			~Material();

			//Update material based current Material::CreateInfo m_CI.
			void Update() override;

		protected:
			bool CreateInfoHasChanged(const ObjectComponentInterface::CreateInfo* pCreateInfo) override;

		public:
			void SetDefaultPBRTextures();

		private:
			void InitialiseUB();
			void CreateDefaultColourTextures();

		public:
			inline std::map<TextureType, Ref<graphics::Texture>>& GetTextures() { return m_CI.pbrTextures; }
			inline const std::map<TextureType, Ref<graphics::Texture>>& GetTextures() const { return m_CI.pbrTextures; }
			inline const Ref<graphics::Uniformbuffer<PBRConstantsUB>>& GetUB() const { return m_UB; }
			inline std::string GetDebugName() const { return "GEAR_CORE_Material: " + m_CI.debugName; }
			inline static graphics::GammaSpace GetGammaSpacePerTextureType(const TextureType& type) { return (type == TextureType::ALBEDO ? graphics::GammaSpace::SRGB : graphics::GammaSpace::LINEAR); }
			inline static bool IsTextureTypeLinear(const TextureType& type) { return (GetGammaSpacePerTextureType(type) == graphics::GammaSpace::LINEAR); }
		};
	}
}