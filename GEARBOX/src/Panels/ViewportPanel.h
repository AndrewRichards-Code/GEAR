#pragma once
#include "Panel.h"
#include "gear_core.h"
#include "../UIContext.h"

namespace gearbox
{
	namespace panels
	{
		class ViewportPanel final : public Panel
		{
			//Methods
		public:
			ViewportPanel();
			~ViewportPanel();

			void Draw(const Ref<gear::graphics::RenderSurface>& renderSurface, const Ref<imgui::UIContext>& context);

			//Members
		private:
			static uint32_t s_ViewportPanelCount;
			uint32_t m_ViewID; //0 is invalid;
			
			ImTextureID m_ImageID = 0;
			ImVec2 m_CurrentSize;
			
			VkSampler m_VKSampler = VK_NULL_HANDLE;
		};
	}
}
