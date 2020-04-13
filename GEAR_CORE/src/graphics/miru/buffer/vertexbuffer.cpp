#include "vertexbuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

using namespace miru;
using namespace miru::crossplatform;

miru::Ref<miru::crossplatform::Context> VertexBuffer::s_Context = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> VertexBuffer::s_MB_CPU_Upload = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> VertexBuffer::s_MB_GPU_Usage = nullptr;

VertexBuffer::VertexBuffer(void* device, const float* data, unsigned int count, unsigned int componentCount)
	:m_Device(device), m_Count(count), m_ComponentCount(componentCount)
{
	InitialiseMemory();

	m_VertexBufferUploadCI.debugName = "GEAR_CORE_VertexBufferUpload";
	m_VertexBufferUploadCI.device = m_Device;
	m_VertexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_VertexBufferUploadCI.size = m_Count * sizeof(float);
	m_VertexBufferUploadCI.data = (void*)data;
	m_VertexBufferUploadCI.pMemoryBlock = s_MB_CPU_Upload;
	m_VertexBufferUpload = Buffer::Create(&m_VertexBufferUploadCI);

	m_VertexBufferCI.debugName = "GEAR_CORE_VertexBufferUsage";
	m_VertexBufferCI.device = m_Device;
	m_VertexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::VERTEX;
	m_VertexBufferCI.size = 0;
	m_VertexBufferCI.data = nullptr;
	m_VertexBufferCI.pMemoryBlock = s_MB_GPU_Usage;
	m_VertexBuffer = Buffer::Create(&m_VertexBufferCI);

	m_VertexBufferViewCI.debugName = "GEAR_CORE_VertexBufferViewUsage";;
	m_VertexBufferViewCI.device = m_Device;
	m_VertexBufferViewCI.type = BufferView::Type::VERTEX;
	m_VertexBufferViewCI.pBuffer = m_VertexBuffer;
	m_VertexBufferViewCI.offset = 0;
	m_VertexBufferViewCI.size = m_Count * sizeof(float);
	m_VertexBufferViewCI.stride = m_ComponentCount * sizeof(float);
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
		mbCI.debugName = "GEAR_CORE_MB_GPU_VertexBuffereUsage";
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
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_VertexBufferUpload, m_VertexBuffer, { {0, 0, m_Count * sizeof(float)} });
		m_Upload = true;
	}
}
