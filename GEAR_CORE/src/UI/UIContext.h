#pragma once
#include "gear_core_common.h"
#include "UI/Panels/BasePanel.h"
#include "UI/Panels/Panels.h"

typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkSampler_T* VkSampler;

struct ID3D12GraphicsCommandList;
struct ID3D12DescriptorHeap;
typedef uint32_t UINT;

namespace gear
{
	namespace asset
	{
		class EditorAssetManager;
	}
	namespace core
	{
		class Timer;
	}
	namespace project
	{
		class Project;
	}
	namespace graphics
	{
		class Window;
	}
	namespace ui
	{
		class MenuBar;

		class GEAR_API UIContext
		{
			//enums/structs
		public:
			struct CreateInfo
			{
				Ref<graphics::Window> window;
			};

			//Methods
		public:
			UIContext(CreateInfo* pCreateInfo);
			~UIContext();

			void Update(core::Timer timer);
			void Draw();
			void RenderDrawData(const miru::base::CommandBufferRef& cmdBuffer, uint32_t frameIndex);

			ImTextureID AddTextureID(const miru::base::ImageViewRef& imageView);
			std::map<miru::base::ImageViewRef, ImTextureID>::iterator RemoveTextureID(const miru::base::ImageViewRef& imageView);
			std::map<miru::base::ImageViewRef, ImTextureID>::iterator RemoveTextureID(const std::map<miru::base::ImageViewRef, ImTextureID>::iterator& it);

			void* GetDevice();
			miru::base::ContextRef GetContext();
			static std::filesystem::path GetSourceDirectory();

		private:
			void Initialise(Ref<graphics::Window>& window);
			void ShutDown();

			void BeginFrame();
			void EndFrame();

			void BeginDockspace();
			void EndDockspace();

			ID3D12GraphicsCommandList* GetID3D12GraphicsCommandList(const miru::base::CommandBufferRef cmdBuffer, uint32_t index);
			VkCommandBuffer GetVkCommandBuffer(const miru::base::CommandBufferRef cmdBuffer, uint32_t index);

			void SetDarkTheme();

		public:
			void SetContentBrowserPanelsFolderpath(const std::filesystem::path& folderpath);

			inline const CreateInfo& GetCreateInfo() { return m_CI; }
			inline Ref<graphics::Window> GetWindow() { return m_CI.window; }
			inline std::vector<Ref<panels::Panel>>& GetEditorPanels() { return m_EditorPanels; };
			inline Ref<project::Project> GetProject() { return m_Project; }
			inline void SetProject(const Ref<project::Project>& project) { m_Project = project; }
			inline Ref<asset::EditorAssetManager> GetEditorAssetManager() { return m_AssetManager; }
			
			static inline UIContext* GetUIContext() { return s_UIContext; }

			template<typename T>
			std::vector<Ref<T>> GetEditorPanelsByType()
			{
				std::vector<Ref<panels::Panel>> selectedPanels;
				for (auto& panel : m_EditorPanels)
				{
					if (panel->GetPanelType() == panels::Panel::Type::UNKNOWN)
					{
						continue;
					}
					if (typeid(T) == typeid(panels::ContentBrowserPanel) && panel->GetPanelType() == panels::Panel::Type::CONTENT_BROWSER)
					{
						selectedPanels.push_back(panel);
						continue;
					}
					if (typeid(T) == typeid(panels::ContentEditorPanel) && panel->GetPanelType() == panels::Panel::Type::CONTENT_EDITOR)
					{
						selectedPanels.push_back(panel);
						continue;
					}
					if (typeid(T) == typeid(panels::SceneHierarchyPanel) && panel->GetPanelType() == panels::Panel::Type::SCENE_HIERARCHY)
					{
						selectedPanels.push_back(panel);
						continue;
					}
					if (typeid(T) == typeid(panels::MaterialPanel) && panel->GetPanelType() == panels::Panel::Type::MATERIAL)
					{
						selectedPanels.push_back(panel);
						continue;
					}
					if (typeid(T) == typeid(panels::OutputPanel) && panel->GetPanelType() == panels::Panel::Type::OUTPUT)
					{
						selectedPanels.push_back(panel);
						continue;
					}
					if (typeid(T) == typeid(panels::ProjectPanel) && panel->GetPanelType() == panels::Panel::Type::PROJECT)
					{
						selectedPanels.push_back(panel);
						continue;
					}
					if (typeid(T) == typeid(panels::PropertiesPanel) && panel->GetPanelType() == panels::Panel::Type::PROPERTIES)
					{
						selectedPanels.push_back(panel);
						continue;
					}
					if (typeid(T) == typeid(panels::RendererPropertiesPanel) && panel->GetPanelType() == panels::Panel::Type::RENDERER_PROPERTIES)
					{
						selectedPanels.push_back(panel);
						continue;
					}
					if (typeid(T) == typeid(panels::ViewportPanel) && panel->GetPanelType() == panels::Panel::Type::VIEWPORT)
					{
						selectedPanels.push_back(panel);
						continue;
					}
				}

				if (selectedPanels.empty())
					selectedPanels.push_back(nullptr);

				std::vector<Ref<T>> _selectedPanels;
				for (auto& panel : selectedPanels)
					_selectedPanels.push_back(ref_cast<T>(panel));

				return _selectedPanels;
			}

			template<typename T>
			std::string GetUniqueIDString(const std::string& label, T* _this)
			{
				size_t idx = 0;
				for (auto& panel : GetEditorPanelsByType<T>())
				{
					if (panel.get() == _this)
						break;

					idx++;
				}
				size_t structHash = typeid(T).hash_code();
				std::string uid_str = label + std::string("##") + std::to_string(structHash) + "_" + std::to_string(idx);
				return uid_str;
			}
	
			//Members
		private:
			CreateInfo m_CI;

			miru::base::GraphicsAPI::API m_API;
			std::vector<Ref<panels::Panel>> m_EditorPanels;
			Ref<MenuBar> m_MenuBar;
			Ref<asset::EditorAssetManager> m_AssetManager;
			Ref<project::Project> m_Project;
			
			static UIContext* s_UIContext;

		public:
			VkDescriptorPool m_VulkanDescriptorPool;
			VkSampler m_VulkanSampler;
			uint32_t m_VulkanDescriptorCount = 0;
			
			ID3D12DescriptorHeap* m_D3D12DescriptorHeapSRV;
			std::map<ImTextureID, UINT> m_D3D12GPUHandleHeapOffsets;
			UINT m_GPUHandleHeapIndex = 1;

			std::map<miru::base::ImageViewRef, ImTextureID> m_TextureIDs;
			std::map<miru::base::ImageViewRef, ImTextureID> m_TextureIDs_PF;
		};
	}
}