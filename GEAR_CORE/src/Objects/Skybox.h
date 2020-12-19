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
			float						gamma = 2.2f;
		};

	private:
		gear::Ref<Model> m_Model;
		Model::CreateInfo m_ModelCI;

		gear::Ref<Mesh> m_Mesh;
		Mesh::CreateInfo m_MeshCI;

		gear::Ref<Material> m_Material;
		Material::CreateInfo m_MaterialCI;

		gear::Ref<graphics::Texture> m_Texture;
		graphics::Texture::CreateInfo m_TextureCI;

		gear::Ref<graphics::Texture> m_GeneratedCubemap;
		graphics::Texture::CreateInfo m_GeneratedCubemapCI;

		gear::Ref<graphics::Texture> m_GeneratedDiffuseCubemap;
		graphics::Texture::CreateInfo m_GeneratedDiffuseCubemapCI;

		gear::Ref<graphics::Texture> m_GeneratedSpecularCubemap;
		graphics::Texture::CreateInfo m_GeneratedSpecularCubemapCI;
		
		gear::Ref<graphics::Texture> m_GeneratedSpecularBRDF_LUT;
		graphics::Texture::CreateInfo m_GeneratedSpecularBRDF_LUT_CI;

		typedef graphics::UniformBufferStructures::SkyboxInfo SkyboxInfoUB;
		gear::Ref<graphics::Uniformbuffer<SkyboxInfoUB>> m_UB;
	
	public:
		bool m_Cubemap;
		bool m_HDR;
		bool m_Generated = false;

		CreateInfo m_CI;

	public:
		Skybox(CreateInfo* pCreateInfo);
		~Skybox();

		//Update the skybox from the current state of Skybox::CreateInfo m_CI.
		void Update();

		inline gear::Ref<graphics::Texture>& GetTexture() { return m_Texture; }
		inline const gear::Ref<graphics::Texture>& GetTexture() const { return m_Texture; }

		inline gear::Ref<graphics::Texture>& GetGeneratedCubemap() { return m_GeneratedCubemap; }
		inline const gear::Ref<graphics::Texture>& GetGeneratedCubemap() const { return m_GeneratedCubemap; }

		inline gear::Ref<graphics::Texture>& GetGeneratedDiffuseCubemap() { return m_GeneratedDiffuseCubemap; }
		inline const gear::Ref<graphics::Texture>& GetGeneratedDiffuseCubemap() const { return m_GeneratedDiffuseCubemap; }

		inline gear::Ref<graphics::Texture>& GetGeneratedSpecularCubemap() { return m_GeneratedSpecularCubemap; }
		inline const gear::Ref<graphics::Texture>& GetGeneratedSpecularCubemap() const { return m_GeneratedSpecularCubemap; }

		inline gear::Ref<graphics::Texture>& GetGeneratedSpecularBRDF_LUT() { return m_GeneratedSpecularBRDF_LUT; }
		inline const gear::Ref<graphics::Texture>& GetGeneratedSpecularBRDF_LUT() const { return m_GeneratedSpecularBRDF_LUT; }

		inline const gear::Ref<objects::Model>& GetModel() const { return m_Model; }
		inline const gear::Ref<graphics::Uniformbuffer<SkyboxInfoUB>>& GetUB() const { return m_UB; }

	private:
		void InitialiseUBs();
	};
}
}
