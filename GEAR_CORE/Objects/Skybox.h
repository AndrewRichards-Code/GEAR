#pragma once
#include "Objects/ObjectInterface.h"

namespace gear
{
	namespace asset
	{
		struct ImageAssetDataBuffer;
	}
	namespace graphics
	{
		class Texture;
	}
	namespace objects
	{
		class ModelData;
		class Model;
		class Mesh;

		class GEAR_OBJECTS_API Skybox : public ObjectInterface, public asset::Asset
		{
		public:
			struct CreateInfo : public ObjectInterface::CreateInfo
			{
				Ref<ModelData>						modelData;
				Ref<asset::ImageAssetDataBuffer>	textureData;
				uint32_t							generatedCubemapSize;
			};

		private:
			Ref<Model> m_Model;
			Ref<Mesh> m_Mesh;
			Ref<asset::ImageAssetDataBuffer> m_TextureData;

			Ref<graphics::Texture> m_HDRTexture;
			Ref<graphics::Texture> m_GeneratedCubemap;
			Ref<graphics::Texture> m_GeneratedDiffuseCubemap;
			Ref<graphics::Texture> m_GeneratedSpecularCubemap;
			Ref<graphics::Texture> m_GeneratedSpecularBRDF_LUT;

		public:
			bool m_Generated = false;

			CreateInfo m_CI;

		public:
			Skybox(CreateInfo* pCreateInfo);
			~Skybox();

			//Update the skybox from the current state of Skybox::CreateInfo m_CI.
			void Update(const Transform& transform) override;

		protected:
			bool CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo) override;

		public:
			inline Ref<graphics::Texture>& GetHDRTexture() { return m_HDRTexture; }
			inline const Ref<graphics::Texture>& GetHDRTexture() const { return m_HDRTexture; }

			inline Ref<graphics::Texture>& GetGeneratedCubemap() { return m_GeneratedCubemap; }
			inline const Ref<graphics::Texture>& GetGeneratedCubemap() const { return m_GeneratedCubemap; }

			inline Ref<graphics::Texture>& GetGeneratedDiffuseCubemap() { return m_GeneratedDiffuseCubemap; }
			inline const Ref<graphics::Texture>& GetGeneratedDiffuseCubemap() const { return m_GeneratedDiffuseCubemap; }

			inline Ref<graphics::Texture>& GetGeneratedSpecularCubemap() { return m_GeneratedSpecularCubemap; }
			inline const Ref<graphics::Texture>& GetGeneratedSpecularCubemap() const { return m_GeneratedSpecularCubemap; }

			inline Ref<graphics::Texture>& GetGeneratedSpecularBRDF_LUT() { return m_GeneratedSpecularBRDF_LUT; }
			inline const Ref<graphics::Texture>& GetGeneratedSpecularBRDF_LUT() const { return m_GeneratedSpecularBRDF_LUT; }

			inline const Ref<objects::Model>& GetModel() const { return m_Model; }
		};
	}
}
