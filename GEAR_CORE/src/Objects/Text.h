#pragma once
#include "gear_core_common.h"
#include "ObjectInterface.h"
#include "Graphics/RenderPipeline.h"
#include "Core/FontLibrary.h"
#include "Objects/Camera.h"
#include "Objects/Model.h"

namespace gear 
{
namespace objects 
{
	class GEAR_API Text : public ObjectInterface
	{
	public:
		struct CreateInfo : public ObjectInterface::CreateInfo
		{
			uint32_t	viewportWidth;
			uint32_t	viewportHeight;
		};

	private:
		struct Line
		{
			Ref<core::FontLibrary::Font>	font;
			std::string						text;
			mars::uint2						initPosition;
			mars::uint2						position;
			mars::float4					colour;
			mars::float4					backgroundColour;
			Ref<Model>						model;
		};
		std::vector<Line> m_Lines;
		
		Ref<Camera> m_Camera;
		Camera::CreateInfo m_CameraCI;
		
	public:
		CreateInfo m_CI;
	
	public:
		Text(CreateInfo* pCreateInfo);
		~Text();
		void AddLine(const Ref<core::FontLibrary::Font>& font, const std::string& text, const mars::uint2& position, const mars::float4& colour,
			const mars::float4& backgroudColour = mars::float4(0.0f, 0.0f, 0.0f, 0.5f));
		void UpdateLine(const std::string& text, size_t lineIndex, bool force = true);

		inline const std::vector<Line>& GetLines() const { return m_Lines; }
		inline const Ref<Camera>& GetCamera() const { return m_Camera; }

		//Update the text from the current state of Text::CreateInfo m_CI.
		void Update(const Transform& transform) override;
	
	private:
		void GenerateLine(size_t lineIndex, bool update = false);
	};
}
}