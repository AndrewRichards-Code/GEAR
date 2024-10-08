#include "gear_core_common.h"
#include "Objects/Probe.h"

using namespace gear;
using namespace objects;
using namespace graphics;

using namespace miru;
using namespace base;

using namespace mars;

Probe::Probe(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseUB();
	InitialiseTextures();

	Update(Transform());
}

Probe::~Probe()
{
	if (m_CI.viewCamera)
	{
		m_CI.viewCamera->RemoveCallback(this);
	}
}

void Probe::Update(const Transform& transform)
{
	//Get View Camera Details
	std::vector<Frustum> viewCameraSubFrusta;
	float viewCameraNear;
	float viewCameraFar;

	static bool setCallback = true;
	if (m_CI.viewCamera && setCallback)
	{
		m_CI.viewCamera->SetCallback(this, (ObjectUpdatedCallback)(&Probe::UpdateFromViewCamera));
		setCallback = false;
	}

	if (CreateInfoHasChanged(&m_CI) || m_ViewCameraUpdated)
	{
		//Textures and Pipeline
		InitialiseTextures();

		CalculateViewCameraSubFrusta(viewCameraSubFrusta, viewCameraNear, viewCameraFar);

		//Projection
		for (uint32_t i = 0; i < m_CI.shadowCascades; i++)
		{
			if (m_CI.directionType == DirectionType::OMNI)
				m_CI.projectionType = Camera::ProjectionType::PERSPECTIVE;

			if (m_CI.projectionType == Camera::ProjectionType::PERSPECTIVE)
			{
				m_UB->proj[i] = float4x4::Perspective(
					m_CI.directionType == DirectionType::OMNI ? DegToRad(90.0) : m_CI.perspectiveHorizonalFOV,
					1.0f,
					m_CI.zNear,
					m_CI.zFar,
					true, 
					false);
			}
			else
			{
				const BoundingBox& extents = viewCameraSubFrusta[i].GetExtents();

				m_UB->proj[i] = float4x4::Orthographic(
					extents.min.x,
					extents.max.x,
					extents.min.y,
					extents.max.y,
					0.0f,
					extents.max.z - extents.min.z,
					true, 
					false);
			}

			if (miru::base::GraphicsAPI::IsVulkan())
				m_UB->proj[i].f *= -1;
		}
	
		m_UB->SubmitData();
	}
	if (TransformHasChanged(transform) || m_ViewCameraUpdated)
	{
		CalculateViewCameraSubFrusta(viewCameraSubFrusta, viewCameraNear, viewCameraFar);

		//View
		{
			if (m_CI.directionType == DirectionType::MONO)
			{
				if (m_CI.projectionType == Camera::ProjectionType::ORTHOGRAPHIC)
				{
					for (uint32_t i = 0; i < m_CI.shadowCascades; i++)
					{
						Transform orthoTransform = transform;

						const float4x4& orientation = transform.orientation.ToRotationMatrix4<float>();
						float3 lightDirection = (-float3(orientation * float4(0, 0, 1, 0))).Normalise();

						const float3& centre = viewCameraSubFrusta[i].GetCentre();
						const BoundingBox& extents = viewCameraSubFrusta[i].GetExtents();

						orthoTransform.translation = centre + lightDirection * abs(extents.min.z);
						m_UB->view[i] = TransformToMatrix4(orthoTransform).Inverse();
					}
				}
				else
				{
					m_UB->view[0] = TransformToMatrix4(transform).Inverse();
				}
				m_UB->cubemap = 0;
			}
			else
			{
				for (uint32_t faceIndex = 0; faceIndex < 6; faceIndex++)
				{
					m_UB->view[faceIndex] = Camera::GetCubemapFaceViewMatrix(faceIndex, transform.translation);
				}
				m_UB->cubemap = 1;
			}
		}

		//Position
		{
			m_UB->position = float4(transform.translation, 1.0f);
		}

		m_UB->farPlanes.x = viewCameraNear + m_CI.shadowCascadeDistances[0];
		m_UB->farPlanes.y = viewCameraNear + m_CI.shadowCascadeDistances[1];
		m_UB->farPlanes.z = viewCameraNear + m_CI.shadowCascadeDistances[2];
		m_UB->farPlanes.w = viewCameraNear + m_CI.shadowCascadeDistances[3];
		m_UB->shadowCascades = m_CI.shadowCascades;

		m_UB->SubmitData();
	}

	//Reset the m_ViewCameraUpdated for the next update from the view camera
	if (m_ViewCameraUpdated)
	{
		m_ViewCameraUpdated = false;
		SetUpdateGPUFlag(); //Ensure that the probe UB is updated
	}
}

void Probe::UpdateFromViewCamera()
{
	m_ViewCameraUpdated = true;
}

float Probe::CalculateCascadeDistance(uint32_t i)
{
	if (!m_CI.viewCamera)
		return 0.0f;

	//https://github.com/SaschaWillems/Vulkan/blob/ba8fea35ac27f02b62b4f1aa93ae0c3262ee8852/examples/shadowmappingcascade/shadowmappingcascade.cpp#L634
	float minZ = m_CI.viewCamera->m_CI.perspectiveParams.zNear;
	float maxZ = m_CI.viewCamera->m_CI.perspectiveParams.zFar;
	float ratio = maxZ / minZ;
	float range = maxZ - minZ;

	// Calculate split depths based on view camera frustum
	// Based on method presented in https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch10.html
	float p = (i + 1) / static_cast<float>(m_CI.shadowCascades);
	float log = minZ * std::pow(ratio, p);
	float uniform = minZ + range * p;
	float d = m_CI.shadowCascadeSplitLambda * (log - uniform) + uniform;
	return (d - minZ);
}

bool Probe::CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	newHash ^= core::GetHash(CI.directionType);
	newHash ^= core::GetHash(CI.captureType);
	newHash ^= core::GetHash(CI.imageSize);
	newHash ^= core::GetHash(CI.projectionType);
	newHash ^= core::GetHash(CI.perspectiveHorizonalFOV);
	newHash ^= core::GetHash(CI.zNear);
	newHash ^= core::GetHash(CI.zFar);
	newHash ^= core::GetHash(CI.viewCamera);
	newHash ^= core::GetHash(CI.shadowCascades);
	newHash ^= core::GetHash(CI.shadowCascadeDistances[0]);
	newHash ^= core::GetHash(CI.shadowCascadeDistances[1]);
	newHash ^= core::GetHash(CI.shadowCascadeDistances[2]);
	newHash ^= core::GetHash(CI.shadowCascadeDistances[3]);
	newHash ^= core::GetHash(CI.shadowCascadeSplitLambda);
	newHash ^= core::GetHash(CI.calculateShadowCascadeDistances);
	return CompareCreateInfoHash(newHash);
}

void Probe::InitialiseTextures()
{
	if (m_CurrentDirectionType != m_CI.directionType
		|| m_CurrentCaptureType != m_CI.captureType
		|| m_CurrentImageSize != m_CI.imageSize)
	{
		if (m_CI.captureType == CaptureType::REFLECTION)
		{
			CreateTexture(m_ColourTexture);
		}
		CreateTexture(m_DepthTexture, false);

		m_CurrentDirectionType = m_CI.directionType;
		m_CurrentCaptureType = m_CI.captureType;
		m_CurrentImageSize = m_CI.imageSize;
	}
}

void Probe::CreateTexture(Ref<Texture>& texture, bool colour)
{
	//TODO: Add mips and multisampling
	Texture::CreateInfo textureCI;
	textureCI.debugName = (colour ? "GEAR_CORE_Texture_Colour: " : "GEAR_CORE_Texture_Depth: ") + m_CI.debugName;
	textureCI.device = m_CI.device;
	textureCI.dataType = Texture::DataType::DATA;
	textureCI.data.data = nullptr;
	textureCI.data.size = 0;
	textureCI.data.width = m_CI.imageSize;
	textureCI.data.height = m_CI.imageSize;
	textureCI.data.depth = 1;
	textureCI.mipLevels = 1;
	textureCI.arrayLayers = (m_CI.directionType == DirectionType::OMNI ? 6 : 1) * std::min(MaxShadowCascades, m_CI.shadowCascades);
	textureCI.type = m_CI.directionType == DirectionType::OMNI ? Image::Type::TYPE_CUBE : Image::Type::TYPE_2D_ARRAY;
	textureCI.format = m_CI.captureType == CaptureType::REFLECTION ? Image::Format::R32G32B32A32_SFLOAT : Image::Format::D32_SFLOAT;
	textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	textureCI.usage = Image::UsageBit::SAMPLED_BIT | (m_CI.captureType == CaptureType::REFLECTION ? Image::UsageBit::COLOUR_ATTACHMENT_BIT : Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT);
	textureCI.generateMipMaps = false;
	textureCI.gammaSpace = GammaSpace::LINEAR;
	texture = CreateRef<Texture>(&textureCI);

	Sampler::CreateInfo samplerCI = texture->GetSampler()->GetCreateInfo();
	samplerCI.addressModeU = miru::base::Sampler::AddressMode::CLAMP_TO_BORDER;
	samplerCI.addressModeV = miru::base::Sampler::AddressMode::CLAMP_TO_BORDER;
	samplerCI.addressModeW = miru::base::Sampler::AddressMode::CLAMP_TO_BORDER;
	samplerCI.borderColour = miru::base::Sampler::BorderColour::FLOAT_OPAQUE_BLACK;
	texture->SetSampler(&samplerCI);
}

void Probe::InitialiseUB()
{
	if (!m_UB)
	{
		ProbeInfoUB zero = { 0 };

		Uniformbuffer<ProbeInfoUB>::CreateInfo ubCI;
		ubCI.debugName = GenereateDebugName<Uniformbuffer<ProbeInfoUB>>(m_CI.debugName);
		ubCI.device = m_CI.device;
		ubCI.data = &zero;
		m_UB = CreateRef<Uniformbuffer<ProbeInfoUB>>(&ubCI);
	}
}

void Probe::CalculateViewCameraSubFrusta(std::vector<Frustum>& viewCameraSubFrusta, float& viewCameraNear, float& viewCameraFar)
{
	viewCameraSubFrusta.clear();
	viewCameraNear = 0.0f;
	viewCameraFar = 1.0f;

	for (uint32_t i = 0; i < m_CI.shadowCascades; i++)
	{
		if (m_CI.viewCamera)
		{
			const Ref<Uniformbuffer<UniformBufferStructures::Camera>>& viewCameraUB = m_CI.viewCamera->GetCameraUB();
			viewCameraNear = m_CI.viewCamera->m_CI.perspectiveParams.zNear;
			viewCameraFar = m_CI.viewCamera->m_CI.perspectiveParams.zFar;
			const float& viewCameraClipRange = viewCameraFar - viewCameraNear;

			if (m_CI.calculateShadowCascadeDistances)
			{
				m_CI.shadowCascadeDistances[i] = CalculateCascadeDistance(i);
			}

			Frustum& frustum = viewCameraSubFrusta.emplace_back(viewCameraUB->proj, viewCameraUB->view);
			const float& near = i == 0 ? 0.0f : m_CI.shadowCascadeDistances[i - 1];
			const float& far = m_CI.shadowCascadeDistances[i];
			frustum.ScaleDistancesForNearAndFar(near / viewCameraClipRange, far / viewCameraClipRange);
		}
		else
		{
			viewCameraSubFrusta.emplace_back();
		}
	}
}
