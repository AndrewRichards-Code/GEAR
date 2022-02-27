#include "gear_core_common.h"
#include "ModelComponentUI.h"

#include "Core/AssetFile.h"
#include "Scene/Entity.h"

using namespace gear;
using namespace scene;
using namespace objects;

using namespace ui;
using namespace componentui;

using namespace mars;

void gear::ui::componentui::DrawModelComponentUI(Entity entity, UIContext* uiContext)
{
	Ref<Model>& model = entity.GetComponent<ModelComponent>().model;
	Model::CreateInfo& CI = model->m_CI;
	
	DrawInputText("Name", CI.debugName);
	DrawMeshUI(CI.pMesh, uiContext);
	DrawFloat("Scaling X", CI.materialTextureScaling.x, 0.0f);
	DrawFloat("Scaling Y", CI.materialTextureScaling.y, 0.0f);
	DrawInputText("Render Pipeline", CI.renderPipelineName);

	model->Update(entity.GetComponent<TransformComponent>());
}

void gear::ui::componentui::AddModelComponent(Entity entity, void* device)
{
	if (!entity.HasComponent<ModelComponent>())
	{
		Mesh::CreateInfo meshCI;
		meshCI.debugName = "Mesh-Cube";
		meshCI.device = device;
		meshCI.filepath = "res/obj/cube.fbx";
		Model::CreateInfo modelCI;
		modelCI.debugName = "Model-Cube";
		modelCI.device = device;
		modelCI.pMesh = CreateRef<Mesh>(&meshCI);
		modelCI.renderPipelineName = "PBROpaque";
		entity.AddComponent<ModelComponent>(&modelCI);

		Transform& transform = entity.GetComponent<TransformComponent>();
		transform.translation = float3(0, 0, 0);
		transform.orientation = Quaternion(1, 0, 0, 0);
		transform.scale = float3(1.0f, 1.0f, 1.0f);
	}
}

void gear::ui::componentui::DrawMeshUI(Ref<Mesh>& mesh, UIContext* uiContext)
{
	Mesh::CreateInfo& CI = mesh->m_CI;

	if (DrawTreeNode("Mesh", false))
	{
		DrawInputText("Filepath", CI.filepath);
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".fbx");
			if (payload)
			{
				std::string filepath = (char*)payload->Data;
				if (std::filesystem::exists(filepath))
				{
					CI.filepath = filepath;
				}
			}
		}
		auto& data = mesh->GetModelData();
		
		size_t id = 1;
		if (DrawTreeNode("Sub Meshes: " + std::to_string(data.meshes.size()), false))
		{
			for (size_t i = 0; i < data.meshes.size(); i++)
			{
				auto& subMesh = data.meshes[i];
				if (DrawTreeNode("Sub Mesh: " + std::to_string(i), false, (void*)id++))
				{
					DrawStaticText("Name", subMesh.meshName);
					DrawStaticNumber("Vertices", subMesh.vertices.size());
					DrawStaticNumber("Indices", subMesh.indices.size());
					DrawStaticNumber("Bones", subMesh.bones.size());
					DrawMaterialUI(mesh->GetMaterial(i), uiContext);
					EndDrawTreeNode();
				}
			}
			EndDrawTreeNode();
		}
		if (DrawTreeNode("Animations: " + std::to_string(data.animations.size()), false))
		{
			for (size_t i = 0; i < data.animations.size(); i++)
			{
				const auto& animation = data.animations[i];
				if (DrawTreeNode("Animation: " + std::to_string(i), false, (void*)id++))
				{
					DrawEnum("Type", { "Animation", "Audio" }, animation.sequenceType);
					DrawStaticNumber("Duration", animation.duration);
					DrawStaticNumber("Frames Per Second", animation.framesPerSecond);

					for (size_t j = 0; j < animation.nodeAnimations.size(); j++)
					{
						const auto& nodeAnimation = animation.nodeAnimations[j];
						if (DrawTreeNode("Node Animation: " + std::to_string(j), false, (void*)id++))
						{
							DrawStaticText("Name", nodeAnimation.name);
							DrawEnum("Type", { "Translation", "Rotation", "Scale" }, nodeAnimation.type);
							DrawStaticNumber("Duration", nodeAnimation.keyframes.size());
							EndDrawTreeNode();
						}
					}
					EndDrawTreeNode();
				}
			}
			EndDrawTreeNode();
		}
	
		mesh->Update();

		EndDrawTreeNode();
	}
}

void gear::ui::componentui::DrawMaterialUI(Ref<objects::Material>& material, UIContext* uiContext, bool fileFunctions)
{
	if (DrawTreeNode("Material", false))
	{
		Material::CreateInfo& CI = material->m_CI;
		DrawInputText("Name", CI.debugName);

		auto MaterialTextureTypeToString = [](const Material::TextureType& type) -> std::string
		{
			switch (type)
			{
			default:
			case Material::TextureType::UNKNOWN:
				return "Unknown";
			case Material::TextureType::NORMAL:
				return "Normal";
			case Material::TextureType::ALBEDO:
				return "Albedo";
			case Material::TextureType::METALLIC:
				return "Metallic";
			case Material::TextureType::ROUGHNESS:
				return "Roughness";
			case Material::TextureType::AMBIENT_OCCLUSION:
				return "Ambient Occlusion";
			case Material::TextureType::EMISSIVE:
				return "Emissive";
			}
		};

		auto DrawPBRTexture = [&](const Material::TextureType& type, Ref<graphics::Texture>& texture, float width = DefaultWidth)
		{
			ImTextureID textureID = GetTextureID(texture->GetTextureImageView(), uiContext, false);

			BeginColumnLabel(MaterialTextureTypeToString(type), width);
			float iconSize = ImGui::CalcItemWidth();
			ImGui::ImageButton(textureID, ImVec2(iconSize, iconSize));
			EndColumnLabel();

			if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight))
			{
				if (ImGui::MenuItem("Reset Texture"))
				{
					material->m_CI.pbrTextures[type] = nullptr;
					material->SetDefaultPBRTextures();
				}

				ImGui::EndPopup();
			}

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".png");
				if (payload)
				{
					std::string filepath = (char*)payload->Data;
					if (std::filesystem::exists(filepath))
					{
						bool linear = false;
						graphics::Texture::CreateInfo textureCI;
						textureCI.debugName = filepath;
						textureCI.device = uiContext->GetDevice();
						textureCI.dataType = graphics::Texture::DataType::FILE;
						textureCI.file.filepaths.push_back(filepath);
						textureCI.mipLevels = graphics::Texture::MaxMipLevel;
						textureCI.arrayLayers = 1;
						textureCI.type = miru::crossplatform::Image::Type::TYPE_2D;
						textureCI.format = linear ? miru::crossplatform::Image::Format::R32G32B32A32_SFLOAT : miru::crossplatform::Image::Format::R8G8B8A8_UNORM;
						textureCI.samples = miru::crossplatform::Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
						textureCI.usage = miru::crossplatform::Image::UsageBit(0);
						textureCI.generateMipMaps = true;
						textureCI.gammaSpace = linear ? graphics::GammaSpace::LINEAR : graphics::GammaSpace::SRGB;
						texture = CreateRef<graphics::Texture>(&textureCI);
					}
				}
			}
		};

		DrawStaticText("Textures", "");
		for (auto& texture : material->GetTextures())
		{
			DrawPBRTexture(texture.first, texture.second);
		}

		DrawStaticText("Colour Tints", "");
		DrawColourPicker4("Fresnel", CI.pbrConstants.fresnel);
		DrawColourPicker4("Albedo", CI.pbrConstants.albedo);
		DrawFloat("Metallic", CI.pbrConstants.metallic, 0.0f, 1.0f);
		DrawFloat("Roughness", CI.pbrConstants.roughness, 0.0f, 1.0f);
		DrawFloat("Ambient Occlusion", CI.pbrConstants.ambientOcclusion, 0.0f, 1.0f);
		DrawColourPicker4("Emissive", CI.pbrConstants.emissive);

		CI.pbrConstants.emissive.r = std::clamp(CI.pbrConstants.emissive.r, 0.0f, 1.0f);
		CI.pbrConstants.emissive.g = std::clamp(CI.pbrConstants.emissive.g, 0.0f, 1.0f);
		CI.pbrConstants.emissive.b = std::clamp(CI.pbrConstants.emissive.b, 0.0f, 1.0f);
		CI.pbrConstants.emissive.a = std::clamp(CI.pbrConstants.emissive.a, 0.0f, 1.0f);
		static float emissiveMultiplier = 1.0f;
		DrawFloat("Emissive Multiplier", emissiveMultiplier, 1.0f, 100.0f);
		CI.pbrConstants.emissive *= emissiveMultiplier;

		material->Update();

		if (fileFunctions)
		{
			DrawStaticText("Filepath", material->m_CI.filepath);
			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".gaf");
				if (payload)
				{
					std::string filepath = (char*)payload->Data;
					if (std::filesystem::exists(filepath))
					{
						core::AssetFile assetFile(filepath);
						material->LoadFromAssetFile(assetFile);
					}
				}
			}
			if (ImGui::Button("Open"))
			{
				std::string filepath = core::AssetFile::FileDialog_Open();
				if (!filepath.empty())
				{
					core::AssetFile assetFile(filepath);
					material->LoadFromAssetFile(assetFile);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save"))
			{
				std::string filepath = core::AssetFile::FileDialog_Save();
				if (!filepath.empty())
				{
					core::AssetFile assetFile(filepath);
					material->SaveToAssetFile(assetFile);
					assetFile.Save();
				}
			}
		}
		EndDrawTreeNode();
	}
}