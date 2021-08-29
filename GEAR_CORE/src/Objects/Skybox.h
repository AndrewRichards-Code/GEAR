#pragma once

#include "Model.h"

namespace gear
{
namespace objects
{
	class Skybox
	{
	public:
		struct CreateInfo
		{
			std::string					debugName;
			void*						device;
			std::vector<std::string>	filepaths;
			uint32_t					generatedCubemapSize;
			Transform					transform;
			float						exposure = 1.0f;
			graphics::ColourSpace		gammaSpace = graphics::ColourSpace::SRGB;
		};

	private:
		Ref<Model> m_Model;
		Model::CreateInfo m_ModelCI;

		Ref<Mesh> m_Mesh;
		Mesh::CreateInfo m_MeshCI;

		Ref<Material> m_Material;
		Material::CreateInfo m_MaterialCI;

		Ref<graphics::Texture> m_Texture;
		graphics::Texture::CreateInfo m_TextureCI;

		Ref<graphics::Texture> m_GeneratedCubemap;
		graphics::Texture::CreateInfo m_GeneratedCubemapCI;

		Ref<graphics::Texture> m_GeneratedDiffuseCubemap;
		graphics::Texture::CreateInfo m_GeneratedDiffuseCubemapCI;

		Ref<graphics::Texture> m_GeneratedSpecularCubemap;
		graphics::Texture::CreateInfo m_GeneratedSpecularCubemapCI;
		
		Ref<graphics::Texture> m_GeneratedSpecularBRDF_LUT;
		graphics::Texture::CreateInfo m_GeneratedSpecularBRDF_LUT_CI;

		typedef graphics::UniformBufferStructures::HDRInfo HDRInfoUB;
		Ref<graphics::Uniformbuffer<HDRInfoUB>> m_UB;
	
	public:
		bool m_Cubemap;
		bool m_HDR;
		bool m_Generated = false;
		bool m_Reload = false;

		CreateInfo m_CI;

	public:
		Skybox(CreateInfo* pCreateInfo);
		~Skybox();

		//Update the skybox from the current state of Skybox::CreateInfo m_CI.
		void Update();

		inline Ref<graphics::Texture>& GetTexture() { return m_Texture; }
		inline const Ref<graphics::Texture>& GetTexture() const { return m_Texture; }

		inline Ref<graphics::Texture>& GetGeneratedCubemap() { return m_GeneratedCubemap; }
		inline const Ref<graphics::Texture>& GetGeneratedCubemap() const { return m_GeneratedCubemap; }

		inline Ref<graphics::Texture>& GetGeneratedDiffuseCubemap() { return m_GeneratedDiffuseCubemap; }
		inline const Ref<graphics::Texture>& GetGeneratedDiffuseCubemap() const { return m_GeneratedDiffuseCubemap; }

		inline Ref<graphics::Texture>& GetGeneratedSpecularCubemap() { return m_GeneratedSpecularCubemap; }
		inline const Ref<graphics::Texture>& GetGeneratedSpecularCubemap() const { return m_GeneratedSpecularCubemap; }

		inline Ref<graphics::Texture>& GetGeneratedSpecularBRDF_LUT() { return m_GeneratedSpecularBRDF_LUT; }
		inline const Ref<graphics::Texture>& GetGeneratedSpecularBRDF_LUT() const { return m_GeneratedSpecularBRDF_LUT; }

		inline const Ref<objects::Model>& GetModel() const { return m_Model; }
		inline const Ref<graphics::Uniformbuffer<HDRInfoUB>>& GetUB() const { return m_UB; }

	private:
		void InitialiseUBs();
	};
}
}
