#pragma once
#include "UI/Panels/BasePanel.h"

namespace gear
{
	namespace graphics
	{
		class RenderPipeline;
	}
	namespace objects
	{
		class Material;
	}
	namespace ui
	{
		namespace panels
		{
			class GEAR_API MaterialPanel final : public Panel
			{
				//enums/structs
			public:

				//Methods
			public:
				MaterialPanel();
				~MaterialPanel();

				void Draw() override;

				inline void SetSelectedMaterial(const Ref<objects::Material>& material) { m_SelectedMaterial = material; }
				inline void SetSelectedRenderPipline(Ref<graphics::RenderPipeline>& renderPipeline) { m_SelectedRenderPipeline = renderPipeline; }

				//Members
			private:
				Ref<objects::Material> m_SelectedMaterial = nullptr;
				Ref<graphics::RenderPipeline> m_SelectedRenderPipeline = nullptr;
			};
		}
	}
}
