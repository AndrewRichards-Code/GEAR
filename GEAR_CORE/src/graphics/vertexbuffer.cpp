#include "gear_core_common.h"
#include "vertexbuffer.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

miru::Ref<miru::crossplatform::Context> VertexBuffer::s_Context = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> VertexBuffer::s_MB_CPU_Upload = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> VertexBuffer::s_MB_GPU_Usage = nullptr;

VertexBuffer::VertexBuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseMemory();

	m_VertexBufferUploadCI.debugName = "GEAR_CORE_VertexBufferUpload";
	m_VertexBufferUploadCI.device = m_CI.device;
	m_VertexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_VertexBufferUploadCI.size = m_CI.size;
	m_VertexBufferUploadCI.data = m_CI.data;
	m_VertexBufferUploadCI.pMemoryBlock = s_MB_CPU_Upload;
	m_VertexBufferUpload = Buffer::Create(&m_VertexBufferUploadCI);

	m_VertexBufferCI.debugName = "GEAR_CORE_VertexBuffer";
	m_VertexBufferCI.device = m_CI.device;
	m_VertexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::VERTEX;
	m_VertexBufferCI.size = m_CI.size;
	m_VertexBufferCI.data = nullptr;
	m_VertexBufferCI.pMemoryBlock = s_MB_GPU_Usage;
	m_VertexBuffer = Buffer::Create(&m_VertexBufferCI);

	m_VertexBufferViewCI.debugName = "GEAR_CORE_VertexBufferViewUsage";
	m_VertexBufferViewCI.device = m_CI.device;
	m_VertexBufferViewCI.type = BufferView::Type::VERTEX;
	m_VertexBufferViewCI.pBuffer = m_VertexBuffer;
	m_VertexBufferViewCI.offset = 0;
	m_VertexBufferViewCI.size = m_CI.size;
	m_VertexBufferViewCI.stride = GetVertexTypeSize(m_CI.type);
	m_VertexBufferView = BufferView::Create(&m_VertexBufferViewCI);
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::InitialiseMemory()
{
	MemoryBlock::CreateInfo mbCI;
	if (!s_MB_CPU_Upload)
	{
		mbCI.debugName = "GEAR_CORE_MB_CPU_VertexBufferUpload";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_Upload = MemoryBlock::Create(&mbCI);
	}
	if (!s_MB_GPU_Usage)
	{
		mbCI.debugName = "GEAR_CORE_MB_GPU_VertexBuffer";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_Usage = MemoryBlock::Create(&mbCI);
	}
}

void VertexBuffer::Upload(miru::Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_VertexBufferUpload, m_VertexBuffer, { {0, 0, m_CI.size} });
		m_Upload = true;
	}
}

size_t VertexBuffer::GetVertexTypeSize(miru::crossplatform::VertexType type)
{
	switch (type)
	{
	case miru::crossplatform::VertexType::FLOAT:
	case miru::crossplatform::VertexType::INT:
	case miru::crossplatform::VertexType::UINT:
		return 4 * 1;
	case miru::crossplatform::VertexType::VEC2:
	case miru::crossplatform::VertexType::IVEC2:
	case miru::crossplatform::VertexType::UVEC2:
		return 4 * 2;
	case miru::crossplatform::VertexType::VEC3:
	case miru::crossplatform::VertexType::IVEC3:
	case miru::crossplatform::VertexType::UVEC3:
		return 4 * 3;
	case miru::crossplatform::VertexType::VEC4:
	case miru::crossplatform::VertexType::IVEC4:
	case miru::crossplatform::VertexType::UVEC4:
		return 4 * 4;
	case miru::crossplatform::VertexType::DOUBLE:
		return 8 * 1;
	case miru::crossplatform::VertexType::DVEC2:
		return 8 * 2;
	case miru::crossplatform::VertexType::DVEC3:
		return 8 * 3;
	case miru::crossplatform::VertexType::DVEC4:
		return 8 * 4;
	default:
		return 0;
	}
}
