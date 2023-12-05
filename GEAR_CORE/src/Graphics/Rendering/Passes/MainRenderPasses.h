#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace objects { class Light; class Skybox; }

	namespace graphics
	{
		namespace rendering
		{
			class Renderer;

			namespace passes
			{
				class MainRenderPasses
				{
				public:
					static void Clear(Renderer& renderer);
					static void Skybox(Renderer& renderer, Ref<objects::Skybox> skybox);
					static void PBROpaque(Renderer& renderer, Ref<objects::Light> light, Ref<objects::Skybox> skybox);
				};
			}
		}
	}
}