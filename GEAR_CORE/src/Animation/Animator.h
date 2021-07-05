#pragma once

#include "gear_core_common.h"
#include "Core/Sequencer.h"
#include "Animation.h"

namespace gear
{

//Forward Declaration
namespace objects
{
	class Mesh;
}

namespace animation
{
	class Animator final : public core::Sequencer
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

		mars::Vec3 InterpolateTranslation(const mars::Vec3& start, const mars::Vec3& end, float t);
		mars::Quat InterpolateRotation(const mars::Quat& start, const mars::Quat& end, float t);
		mars::Vec3 InterpolateScale(const mars::Vec3& start, const mars::Vec3& end, float t);
	};
}
}
