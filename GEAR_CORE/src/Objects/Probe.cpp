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

	if (m_CI.captureType == CaptureType::REFLECTION)
		CreateTexture(m_ColourTexture, m_ColourTextureCI);

	CreateTexture(m_DepthTexture, m_DepthTextureCI, false);

	CreateRenderPipeline();
}

Probe::~Probe()
{
}

void Probe::Update(const Transform& transform)
{
	if (CreateInfoHasChanged(&m_CI))
	{
		uint64_t newHash = m_CreateInfoHash;
		*this = Probe(&m_CI);
		m_CreateInfoHash = newHash; //Set the Hash value from the previous instance of the Mesh.

		//Projection
		{
			if (m_CI.directionType == DirectionType::OMNI)
				m_CI.projectionType = Camera::ProjectionType::PERSPECTIVE;

			const float& aspectRatio = static_cast<float>(m_CI.imageWidth) / static_cast<float>(m_CI.imageHeight);
			const float orthoScale = 10.0f;
			if (m_CI.projectionType == Camera::ProjectionType::PERSPECTIVE)
			{
				m_UB->proj = float4x4::Perspective(
					m_CI.directionType == DirectionType::OMNI ? DegToRad(90.0) : m_CI.perspectiveHorizonalFOV,
					m_CI.directionType == DirectionType::OMNI ? 1.0f : aspectRatio,
					m_CI.zNear,
					m_CI.zFar);
			}
			else
			{
				m_UB->proj = float4x4::Orthographic(
					m_CI.orthographicScale * -1.0f,
					m_CI.orthographicScale * +1.0f,
					m_CI.orthographicScale * -aspectRatio,
					m_CI.orthographicScale * +aspectRatio,
					m_CI.zNear,
					m_CI.zFar);
			}
			if (miru::base::GraphicsAPI::IsVulkan())
				m_UB->proj.f *= -1;
		}
	}
	if (TransformHasChanged(transform))
	{
		//View
		{
			if (m_CI.directionType == DirectionType::MONO)
			{
				if (m_CI.projectionType == Camera::ProjectionType::ORTHOGRAPHIC)
				{
					Transform orthoTransform = transform;
					orthoTransform.translation = float3();
					m_UB->view[0] = TransformToMatrix4(orthoTransform).Inverse();
				}
				else
				{
					m_UB->view[0] = TransformToMatrix4(transform).Inverse();
				}
			}
			else
			{
				for (uint32_t faceIndex = 0; faceIndex < 6; faceIndex++)
				{
					m_UB->view[faceIndex] = Camera::GetCubemapFaceViewMatrix(faceIndex, transform.translation);
				}
			}
		}

		//Position
		{
			m_UB->position = float4(transform.translation, 1.0f);
		}

		m_UB->SubmitData();
	}
}

bool Probe::CreateInfoHasChanged(const ObjectInterface::CreateInfo* pCreateInfo)
{
	const CreateInfo& CI = *reinterpret_cast<const CreateInfo*>(pCreateInfo);
	uint64_t newHash = 0;
	newHash ^= core::GetHash(CI.directionType);
	newHash ^= core::GetHash(CI.captureType);
	newHash ^= core::GetHash(CI.imageWidth);
	newHash ^= core::GetHash(CI.imageHeight);
	newHash ^= core::GetHash(CI.projectionType);
	newHash ^= core::GetHash(CI.perspectiveHorizonalFOV);
	newHash ^= core::GetHash(CI.orthographicScale);
	newHash ^= core::GetHash(CI.zNear);
	newHash ^= core::GetHash(CI.zFar);
	return CompareCreateInfoHash(newHash);
}

void Probe::CreateTexture(Ref<Texture>& texture, Texture::CreateInfo& textureCI, bool colour)
{
	//TODO: Add mips and multisampling 
	textureCI.debugName = (colour ? "GEAR_CORE_Texture_Colour: " : "GEAR_CORE_Texture_Depth: ") + m_CI.debugName;
	textureCI.device = m_CI.device;
	textureCI.dataType = Texture::DataType::DATA;
	textureCI.data.data = nullptr;
	textureCI.data.size = 0;
	textureCI.data.width = m_CI.imageWidth;
	textureCI.data.height = m_CI.directionType == DirectionType::MONO ? m_CI.imageHeight : m_CI.imageWidth;
	textureCI.data.depth = 1;
	textureCI.mipLevels = 1;
	textureCI.arrayLayers = m_CI.directionType == DirectionType::OMNI ? 6 : 1;
	textureCI.type = m_CI.directionType == DirectionType::OMNI ? Image::Type::TYPE_CUBE : Image::Type::TYPE_2D;
	textureCI.format = m_CI.captureType == CaptureType::REFLECTION ? Image::Format::R32G32B32A32_SFLOAT : Image::Format::D32_SFLOAT;
	textureCI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	textureCI.usage = Image::UsageBit::SAMPLED_BIT | (m_CI.captureType == CaptureType::REFLECTION ? Image::UsageBit::COLOUR_ATTACHMENT_BIT : Image::UsageBit::DEPTH_STENCIL_ATTACHMENT_BIT);
	textureCI.generateMipMaps = false;
	textureCI.gammaSpace = GammaSpace::LINEAR;
	texture = CreateRef<Texture>(&textureCI);
}

void Probe::CreateRenderPipeline()
{
	m_ShadowRenderPipelineLI.device = m_CI.device;
	m_ShadowRenderPipelineLI.filepath = "res/pipelines/Shadow.grpf";
	m_ShadowRenderPipelineLI.samples = Image::SampleCountBit::SAMPLE_COUNT_1_BIT;
	if (m_CI.captureType == CaptureType::REFLECTION)
		m_ShadowRenderPipelineLI.colourAttachmentFormats.push_back(Image::Format::R32G32B32A32_SFLOAT);
	m_ShadowRenderPipelineLI.depthAttachmentFormat = Image::Format::D32_SFLOAT;
	m_ShadowRenderPipeline = CreateRef<RenderPipeline>(&m_ShadowRenderPipelineLI);
}

void Probe::InitialiseUB()
{
	if (!m_UB)
	{
		float zero0[sizeof(ProbeInfoUB)] = { 0 };

		Uniformbuffer<ProbeInfoUB>::CreateInfo ubCI;
		ubCI.debugName = GenereateDebugName<Uniformbuffer<ProbeInfoUB>>(m_CI.debugName);
		ubCI.device = m_CI.device;
		ubCI.data = zero0;
		m_UB = CreateRef<Uniformbuffer<ProbeInfoUB>>(&ubCI);
	}
}

