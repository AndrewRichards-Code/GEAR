#pragma once
#include "gear_core_common.h"
#include "Core/Sequencer.h"
#include "Objects/Transform.h"

namespace gear
{
	namespace objects
	{
		class Mesh;
	}
	namespace animation
	{
		class GEAR_API Animator final : public core::Sequencer
		{
		public:
			struct CreateInfo
			{
				std::string			debugName;
				Ref<objects::Mesh>	pMesh;
			};

			struct AnimationUpdate
			{
				std::string			nodeName;
				objects::Transform	transform;
			};

		public:
			CreateInfo m_CI;

		public:
			Animator(CreateInfo* pCreateInfo);
			~Animator() = default;

			void Update();

		private:
			void Update(const core::Sequence* sequences, size_t sequenceCount) override;

		};
	}
}
