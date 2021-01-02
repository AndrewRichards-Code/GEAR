#pragma once

#include "gear_core_common.h"

#include "Graphics/RenderPipeline.h"
#include "Objects/FontLibrary.h"
#include "Objects/Camera.h"
#include "Objects/Model.h"

namespace gear 
{
namespace objects 
{
	class Text
	{
	public:
		struct CreateInfo
		{
			void*		device;
			uint32_t	viewportWidth;
			uint32_t	viewportHeight;
		};

	private:
		struct Line
		{
			gear::Ref<FontLibrary::Font>		font;
			std::string							text;
			mars::Uint2							initPosition;
			mars::Uint2							position;
			mars::Vec4							colour;
			mars::Vec4							backgroundColour;
			gear::Ref<Model>					model;
		};
		std::vector<Line> m_Lines;
		
		gear::Ref<Camera> m_Camera;
		Camera::CreateInfo m_CameraCI;
		
	public:
		CreateInfo m_CI;
	
	public:
		Text(CreateInfo* pCreateInfo);
		~Text();
		void AddLine(const gear::Ref<FontLibrary::Font>& font, const std::string& text, const mars::Uint2& position, const mars::Vec4& colour, 
			const mars::Vec4& backgroudColour = mars::Vec4(0.0f, 0.0f, 0.0f, 0.5f));
		void UpdateLine(const std::string& text, size_t lineIndex, bool force = true);

		inline const std::vector<Line>& GetLines() const { return m_Lines; }
		inline const gear::Ref<Camera>& GetCamera() const { return m_Camera; }

		//Update the text from the current state of Text::CreateInfo m_CI.
		void Update();
	
	private:
		void GenerateLine(size_t lineIndex, bool update = false);
	};
}
}