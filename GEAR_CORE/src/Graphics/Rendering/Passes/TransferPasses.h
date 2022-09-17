#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace objects { class Camera; class Model; class Light; class Skybox; }

	namespace graphics
	{
		class Texture;

		namespace rendering
		{
			class Renderer;

			namespace passes
			{
				class TransferPasses
				{
				public:
					static void Upload(Renderer& renderer, std::vector<Ref<objects::Camera>>& allCameras, Ref<objects::Skybox> skybox, std::vector<Ref<objects::Light>>& lights, std::vector<Ref<objects::Model>>& allQueue);
				};
			}
		}
	}
}