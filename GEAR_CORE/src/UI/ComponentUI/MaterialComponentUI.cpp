#include "gear_core_common.h"
#include "UI/ComponentUI/MaterialComponentUI.h"
#include "UI/ComponentUI/ComponentUI.h"
#include "UI/UIContext.h"
#include "Graphics/RenderPipeline.h"
#include "Objects/Material.h"

using namespace gear;
using namespace scene;
using namespace objects;
using namespace graphics;
using namespace core;

using namespace ui;
using namespace componentui;

using namespace miru;
using namespace base;

using namespace mars;

void gear::ui::componentui::DrawMaterialUI(Ref<Material>& material, UIContext* uiContext, bool fileFunctions)
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

		auto DrawPBRTexture = [&](const Material::TextureType& type, Ref<Texture>& texture, float width = DefaultWidth)
		{
			ImGui::PushID(texture.get());

			ImTextureID textureID = GetTextureID(texture->GetImageView(), uiContext, false);
			std::string textureTypeStr = MaterialTextureTypeToString(type);

			BeginColumnLabel(textureTypeStr, width);
			const float& iconSize = ImGui::CalcItemWidth();
			ImGui::ImageButton(textureID, ImVec2(iconSize, iconSize));

			GammaSpace& gammaSpace = texture->GetCreateInfo().gammaSpace;
			DrawDropDownMenu("Gamma Space", gammaSpace, DefaultWidth);
			bool linear = gammaSpace == GammaSpace::LINEAR;

			if (ImGui::Button("Reset Texture"))
			{
				texture = nullptr;
				material->SetDefaultPBRTextures();
			}
			EndColumnLabel();

			if (ImGui::BeginDragDropTarget())
			{
				const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(".png");
				if (payload)
				{
					std::string filepath = (char*)payload->Data;
					if (std::filesystem::exists(filepath))
					{
						bool linear = Material::IsTextureTypeLinear(type);
						Texture::CreateInfo textureCI;
						textureCI.debugName = filepath;
						textureCI.device = uiContext->GetDevice();
						textureCI.dataType = Texture::DataType::FILE;
						textureCI.file.filepaths.push_back(filepath);
						textureCI.mipLevels = Texture::MaxMipLevel;
						textureCI.arrayLayers = 1;
						textureCI.type = Image::Type::TYPE_2D;
						textureCI.format = linear ? Image::Format::R32G32B32A32_SFLOAT : Image::Format::R8G8B8A8_UNORM;
						textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
						textureCI.usage = Image::UsageBit(0);
						textureCI.generateMipMaps = true;
						textureCI.gammaSpace = linear ? GammaSpace::LINEAR : GammaSpace::SRGB;
						texture = CreateRef<Texture>(&textureCI);
					}
				}
			}
			ImGui::PopID();
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
						AssetFile assetFile(filepath);
						material->LoadFromAssetFile(assetFile);
					}
				}
			}
			if (ImGui::Button("Open"))
			{
				std::string filepath = AssetFile::FileDialog_Open();
				if (!filepath.empty())
				{
					AssetFile assetFile(filepath);
					material->LoadFromAssetFile(assetFile);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("Save"))
			{
				std::string filepath = AssetFile::FileDialog_Save();
				if (!filepath.empty())
				{
					AssetFile assetFile(filepath);
					material->SaveToAssetFile(assetFile);
					assetFile.Save();
				}
			}
		}
		EndDrawTreeNode();
	}
}

void gear::ui::componentui::DrawRenderPipelineUI(RenderPipeline* renderPipeline)
{
	if (DrawTreeNode("Render Pipeline", false))
	{
		RenderPipeline::CreateInfo& CI = renderPipeline->m_CI;

		DrawInputText("Name", CI.debugName);

		size_t id = 1024;
		if (DrawTreeNode("Shaders", false))
		{
			for (auto& shaderCI : CI.shaderCreateInfo)
			{
				DrawInputText("Name", shaderCI.debugName);
				for (auto& stageAndEntryPoint : shaderCI.stageAndEntryPoints)
				{
					DrawDropDownMenu_Bitmask("Stage",
						{
							"VERTEX_BIT",
							"TESSELLATION_CONTROL_BIT",
							"TESSELLATION_EVALUATION_BIT",
							"GEOMETRY_BIT",
							"FRAGMENT_BIT",
							"COMPUTE_BIT",
							"TASK_BIT",
							"MESH_BIT",
							"RAYGEN_BIT",
							"ANY_HIT_BIT",
							"CLOSEST_HIT_BIT",
							"MISS_BIT",
							"INTERSECTION_BIT",
							"CALLABLE_BIT",
						}, stageAndEntryPoint.first);
					ImGui::SameLine();
					DrawInputText("Entry Point", stageAndEntryPoint.second);

					if (DrawTreeNode("Recompile Arguments", false, (void*)id++))
					{
						Shader::CompileArguments& RA = shaderCI.recompileArguments;

						DrawInputText("HLSL Filepath", RA.hlslFilepath);
						DrawInputText("Output Directory", RA.outputDirectory);
						DrawInputVectorOfString("Include Directories", RA.includeDirectories);
						DrawInputText("Entry Point", RA.entryPoint);
						DrawInputText("Shader Model", RA.shaderModel);
						DrawInputVectorOfString("Macros", RA.macros);
						DrawCheckbox("CSO", RA.cso);
						DrawCheckbox("SPV", RA.spv);
						DrawInputVectorOfString("DXC Arguments", RA.dxcArguments);

						EndDrawTreeNode();
					}
				}
			}
			EndDrawTreeNode();
		}
		if (DrawTreeNode("Input Assembly", false))
		{
			Pipeline::InputAssemblyState& ISA = CI.inputAssemblyState;

			DrawDropDownMenu("Topology", ISA.topology);
			DrawCheckbox("Primitive Restart Enable", ISA.primitiveRestartEnable);
			EndDrawTreeNode();
		}
		if (DrawTreeNode("Viewport State", false))
		{
			Pipeline::ViewportState VS = CI.viewportState;
			for (auto& viewport : VS.viewports)
			{
				if (DrawTreeNode("Viewport", false, (void*)id++))
				{
					DrawFloat("X", viewport.x, 0.0f);
					DrawFloat("Y", viewport.y, 0.0f);
					DrawFloat("Width", viewport.width, 0.0f);
					DrawFloat("Height", viewport.height, 0.0f);
					DrawFloat("Min Depth", viewport.minDepth, 0.0f);
					DrawFloat("Max Depth", viewport.maxDepth, 0.0f);
					EndDrawTreeNode();
				}
			}
			for (auto& scissor : VS.scissors)
			{
				if (DrawTreeNode("Scissor", false, (void*)id++))
				{
					DrawInt32("X", scissor.offset.x, INT32_MIN, INT32_MAX);
					DrawInt32("Y", scissor.offset.y, INT32_MIN, INT32_MAX);
					DrawUint32("Width", scissor.extent.width, 0, UINT32_MAX);
					DrawUint32("Height", scissor.extent.height, 0, UINT32_MAX);
					EndDrawTreeNode();
				}
			}
			EndDrawTreeNode();
		}
		if (DrawTreeNode("Rasterisation", false))
		{
			Pipeline::RasterisationState& RS = CI.rasterisationState;

			DrawCheckbox("Depth Clamp Eanble", RS.depthClampEnable);
			DrawCheckbox("Rasteriser Discard Enable", RS.rasteriserDiscardEnable);
			DrawDropDownMenu("Polygon Mode", RS.polygonMode);
			DrawDropDownMenu("Cull Mode", RS.cullMode);
			DrawDropDownMenu("Front Face", RS.frontFace);
			DrawCheckbox("Depth Bias Enable", RS.depthBiasEnable);
			DrawFloat("Depth Bias Constant Factor", RS.depthBiasConstantFactor, 0.0f);
			DrawFloat("Depth Bias Clamp", RS.depthBiasClamp, 0.0f);
			DrawFloat("Depth Bias Slope Factor", RS.depthBiasSlopeFactor, 0.0f);
			DrawFloat("Line Width", RS.lineWidth, 0.0f);
			EndDrawTreeNode();
		}
		if (DrawTreeNode("Multisample", false))
		{
			Pipeline::MultisampleState& MS = CI.multisampleState;

			DrawDropDownMenu("Rasterisation Samples", MS.rasterisationSamples);
			DrawCheckbox("Sample Shading Enable", MS.sampleShadingEnable);
			DrawFloat("Min Sample Shading", MS.minSampleShading, 0.0f);
			DrawUint32("Sample Mask", MS.sampleMask, 0, UINT32_MAX);
			DrawCheckbox("Alpha To Covering Enable", MS.alphaToCoverageEnable);
			DrawCheckbox("Alpha To One Enable", MS.alphaToOneEnable);
			EndDrawTreeNode();
		}
		if (DrawTreeNode("Depth Stencil", false))
		{
			Pipeline::DepthStencilState& DS = CI.depthStencilState;

			DrawCheckbox("Depth Test Enable", DS.depthTestEnable);
			DrawCheckbox("Depth Write Enable", DS.depthWriteEnable);
			DrawDropDownMenu("Depth Compare", DS.depthCompareOp);
			DrawCheckbox("Depth Bounds Test Enable", DS.depthBoundsTestEnable);
			DrawCheckbox("Stencil Test Enable", DS.stencilTestEnable);

			auto DrawStencilOpState = [&](StencilOpState& SOS, bool front) -> void
			{
				std::string name = front ? "Front" : "Back";
				name += " Stencil State";

				if (DrawTreeNode(name, false, (void*)id++))
				{
					DrawDropDownMenu("Fail", SOS.failOp);
					DrawDropDownMenu("Pass", SOS.passOp);
					DrawDropDownMenu("Depth Fail", SOS.depthFailOp);
					DrawDropDownMenu("Compare", SOS.compareOp);
					DrawUint32("Compare Mask", SOS.compareMask, 0, UINT32_MAX);
					DrawUint32("WriteMask", SOS.writeMask, 0, UINT32_MAX);
					DrawUint32("Compare Mask", SOS.reference, 0, UINT32_MAX);
					EndDrawTreeNode();
				}
			};

			DrawStencilOpState(DS.front, true);
			DrawStencilOpState(DS.back, false);
			DrawFloat("Min Depth Bounds", DS.minDepthBounds, 0.0f);
			DrawFloat("Max Depth Bounds", DS.maxDepthBounds, 0.0f);
			EndDrawTreeNode();
		}
		if (DrawTreeNode("Colour Blend", false))
		{
			Pipeline::ColourBlendState& CB = CI.colourBlendState;

			DrawCheckbox("Logic Enable", CB.logicOpEnable);
			DrawDropDownMenu("Logic", CB.logicOp);

			for (auto& CBA : CB.attachments)
			{
				if (DrawTreeNode("Colour Blend Attachment", false, (void*)id++))
				{
					DrawCheckbox("Blend Enable", CBA.blendEnable);
					DrawDropDownMenu("SRC Colour Blend Factor", CBA.srcColourBlendFactor);
					DrawDropDownMenu("DST Colour Blend Factor", CBA.dstColourBlendFactor);
					DrawDropDownMenu("Colour Blend", CBA.colourBlendOp);
					DrawDropDownMenu("SRC Alpha Blend Factor", CBA.srcAlphaBlendFactor);
					DrawDropDownMenu("DST Alpha Blend Factor", CBA.dstAlphaBlendFactor);
					DrawDropDownMenu("Alpha Blend", CBA.alphaBlendOp);
					//DrawDropDownMenu("Colour Write Mask", CBA.colourWriteMask); //TODO: implement for bitmasks.
					EndDrawTreeNode();
				}
			}

			DrawFloat("Blend Constant R", CB.blendConstants[0], 0.0f, 1.0f);
			DrawFloat("Blend Constant G", CB.blendConstants[1], 0.0f, 1.0f);
			DrawFloat("Blend Constant B", CB.blendConstants[2], 0.0f, 1.0f);
			DrawFloat("Blend Constant A", CB.blendConstants[3], 0.0f, 1.0f);
			EndDrawTreeNode();
		}
		EndDrawTreeNode();
	}
}
