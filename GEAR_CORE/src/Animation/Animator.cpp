#include "gear_core_common.h"
#include "Animator.h"
#include "Objects/Mesh.h"
#include "Objects/Transform.h"

using namespace gear;
using namespace animation;
using namespace mars;

Animator::Animator(CreateInfo* pCreateInfo)
	: m_CI(*pCreateInfo)
{
}

void Animator::Update()
{
	const Animation* animations = m_CI.pMesh->GetModelData().animations.data();
	size_t animationCount = m_CI.pMesh->GetModelData().animations.size();

	Update((const core::Sequence*)animations, animationCount);
}

void Animator::Update(const core::Sequence* sequences, size_t sequenceCount)
{
	m_Timer.Update();
	const double& elapsedTime = m_Timer.ElapsedTime();

	for (size_t i = 0; i < sequenceCount; i++)
	{
		const Animation* animations = (const Animation*)(&(sequences[i]));
		if (!animations)
			continue;
		if (animations->sequenceType != core::Sequence::Type::ANIMATION)
			continue;

		std::vector<AnimationUpdate> aus;
		AnimationUpdate au;
		for (auto& nodeAnimation : animations->nodeAnimations)
		{
			for (auto& meshData : m_CI.pMesh->GetModelData().meshes)
			{
				bool found = (meshData.nodeName.compare(nodeAnimation.name) == 0);
				bool inUse = (au.nodeName.compare(nodeAnimation.name) == 0);
				if (found && !inUse)
				{
					if (!au.nodeName.empty())
					{
						aus.push_back(au);
					}
					au.nodeName = nodeAnimation.name;
					au.transform = objects::Transform();
					break;
				}
			}
			if (au.nodeName.empty())
			{
				continue;
			}

			size_t keyframeIdx = ~0;
			const size_t& keyframeCount = nodeAnimation.keyframes.size();
			for (size_t j = 0; j < keyframeCount; j++)
			{
				const auto& kf = nodeAnimation.keyframes[j];
				const double& timepoint = kf.first;
				if (elapsedTime < timepoint && keyframeIdx == ~0)
				{
					continue;
				}
				else
				{
					keyframeIdx = j;
					break;
				}
			}

			const NodeAnimation::Keyframe& kf1 = nodeAnimation.keyframes[(keyframeIdx + 0) % keyframeCount];
			const NodeAnimation::Keyframe& kf2 = nodeAnimation.keyframes[(keyframeIdx + 1) % keyframeCount];

			const double& duration = kf2.first - kf1.first;
			double normalisedLerpPoint = (elapsedTime - kf1.first) / (kf2.first - kf1.first);

			if (nodeAnimation.type == NodeAnimation::Type::TRANSLATION)
			{
				au.transform.translation = InterpolateTranslation(kf1.second.translation, kf2.second.translation, static_cast<float>(normalisedLerpPoint));
			}
			else if (nodeAnimation.type == NodeAnimation::Type::ROTATION)
			{
				au.transform.orientation = InterpolateRotation(kf1.second.orientation, kf2.second.orientation, static_cast<float>(normalisedLerpPoint));
			}
			else if (nodeAnimation.type == NodeAnimation::Type::SCALE)
			{
				au.transform.scale = InterpolateScale(kf1.second.scale, kf2.second.scale, static_cast<float>(normalisedLerpPoint));
			}
			else
			{
				continue;
			}
		}
	}
}

float3 Animator::InterpolateTranslation(const float3& start, const float3& end, float t)
{
	return (start + (end - start) * t);
}

Quaternion Animator::InterpolateRotation(const Quaternion& start, const Quaternion& end, float t)
{
	Quaternion q_start = Quaternion::Normalise(start);
	Quaternion q_end	= Quaternion::Normalise(end);

	double4 v_start(q_start.i, q_start.j, q_start.k, q_start.s);
	double4 v_end(q_end.i, q_end.j, q_end.k, q_end.s);

	float dot = v_start.Dot(v_end);
	if (dot < 0.0f)
	{
		dot *= -1;
		v_end *= -1;
	}
	if (dot < -1.0)
		dot = -1.0;
	if (dot > 1.0)
		dot = 1.0;

	double theta = acos(dot);

	double a = sin((1 - t) * theta);
	double b = sin(t * theta);
	double c = sin(theta);

	Quaternion result(v_start * (a / c) + v_end * (b / c));
	result.Normalise();
	return result;
}

float3 Animator::InterpolateScale(const float3& start, const float3& end, float t)
{
	return (start + (end - start) * t);
}
