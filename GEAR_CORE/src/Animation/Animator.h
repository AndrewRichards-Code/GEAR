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

		mars::float3 InterpolateTranslation(const mars::float3& start, const mars::float3& end, float t);
		mars::Quaternion InterpolateRotation(const mars::Quaternion& start, const mars::Quaternion& end, float t);
		mars::float3 InterpolateScale(const mars::float3& start, const mars::float3& end, float t);
	};
}
}
