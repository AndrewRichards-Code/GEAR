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
		
		bool m_Cubemap;
		bool m_HDR;

		struct SkyboxInfo
		{
			float exposure;
			float gamma;
		};
		gear::Ref<graphics::Uniformbuffer<SkyboxInfo>> m_UB;

	public:
		CreateInfo m_CI;

	public:
		Skybox(CreateInfo* pCreateInfo);
		~Skybox();

		//Update the skybox from the current state of Skybox::CreateInfo m_CI.
		void Update();

		inline const gear::Ref<objects::Model>& GetModel() const { return m_Model; }

		inline const gear::Ref<graphics::Uniformbuffer<SkyboxInfo>>& GetUB() const { return m_UB; }


	private:
		void InitialiseUBs();
	};
}
}
