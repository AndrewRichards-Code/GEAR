#pragma once
#include "ObjectInterface.h"
#include "Model.h"

namespace gear
{
namespace objects
{
	class GEAR_API Skybox : public ObjectInterface
	{
	public:
		struct CreateInfo : public ObjectInterface::CreateInfo
		{
			std::string				filepath;
			uint32_t				generatedCubemapSize;
		};

	private:
		Ref<Model> m_Model;
		Model::CreateInfo m_ModelCI;

		Ref<Mesh> m_Mesh;
		Mesh::CreateInfo m_MeshCI;

		Ref<Material> m_Material;
		Material::CreateInfo m_MaterialCI;

		Ref<graphics::Texture> m_HDRTexture;
		graphics::Texture::CreateInfo m_HDRTextureCI;

		Ref<graphics::Texture> m_GeneratedCubemap;
		graphics::Texture::CreateInfo m_GeneratedCubemapCI;

		Ref<graphics::Texture> m_GeneratedDiffuseCubemap;
		graphics::Texture::CreateInfo m_GeneratedDiffuseCubemapCI;

		Ref<graphics::Texture> m_GeneratedSpecularCubemap;
		graphics::Texture::CreateInfo m_GeneratedSpecularCubemapCI;
		
		Ref<graphics::Texture> m_GeneratedSpecularBRDF_LUT;
		graphics::Texture::CreateInfo m_GeneratedSpecularBRDF_LUT_CI;
	
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
