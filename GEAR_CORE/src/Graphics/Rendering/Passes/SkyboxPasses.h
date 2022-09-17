#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace objects { class Skybox; }

	namespace graphics
	{
		namespace rendering
		{
			class Renderer;

			namespace passes
			{
				class SkyboxPasses
				{
				public:
					static void EquirectangularToCube(Renderer& renderer, Ref<objects::Skybox> skybox);
					static void GenerateMipmaps(Renderer& renderer, Ref<objects::Skybox> skybox);
					static void DiffuseIrradiance(Renderer& renderer, Ref<objects::Skybox> skybox);
					static void SpecularIrradiance(Renderer& renderer, Ref<objects::Skybox> skybox);
					static void SpecularBRDF_LUT(Renderer& renderer, Ref<objects::Skybox> skybox);
				};
			}
		}
	}
}