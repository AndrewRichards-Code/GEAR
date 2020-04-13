#include "indexbuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

using namespace miru;
using namespace miru::crossplatform;

miru::Ref<miru::crossplatform::Context> IndexBuffer::s_Context = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> IndexBuffer::s_MB_CPU_Upload = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> IndexBuffer::s_MB_GPU_Usage = nullptr;

IndexBuffer::IndexBuffer(void* device, const unsigned int* data, unsigned int count)
	:m_Device(device), m_Count(count)
{
	InitialiseMemory();

	m_IndexBufferUploadCI.debugName = "GEAR_CORE_IndexBufferUpload";
	m_IndexBufferUploadCI.device = m_Device;
	m_IndexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_IndexBufferUploadCI.size = m_Count * sizeof(unsigned int);
	m_IndexBufferUploadCI.data = (void*)data;
	m_IndexBufferUploadCI.pMemoryBlock = s_MB_CPU_Upload;
	m_IndexBufferUpload = Buffer::Create(&m_IndexBufferUploadCI);

	m_IndexBufferCI.debugName = "GEAR_CORE_IndexBufferUsage";
	m_IndexBufferCI.device = m_Device;
	m_IndexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::INDEX;
	m_IndexBufferCI.size = 0;
	m_IndexBufferCI.data = nullptr;
	m_IndexBufferCI.pMemoryBlock = s_MB_GPU_Usage;
	m_IndexBuffer = Buffer::Create(&m_IndexBufferCI);

	m_IndexBufferViewCI.debugName = "GEAR_CORE_VertexBufferViewUsage";;
	m_IndexBufferViewCI.device = m_Device;
	m_IndexBufferViewCI.type = BufferView::Type::INDEX;
	m_IndexBufferViewCI.pBuffer = m_IndexBuffer;
	m_IndexBufferViewCI.offset = 0;
	m_IndexBufferViewCI.size = m_Count * sizeof(float);
	m_IndexBufferViewCI.stride =  sizeof(uint32_t);
	m_IndexBufferView = BufferView::Create(&m_IndexBufferViewCI);
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::InitialiseMemory()
{
	MemoryBlock::CreateInfo mbCI;
	if (!s_MB_CPU_Upload)
	{
		mbCI.debugName = "GEAR_CORE_MB_CPU_IndexBufferUpload";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_Upload = MemoryBlock::Create(&mbCI);
	}
	if (!s_MB_GPU_Usage)
	{
		mbCI.debugName = "GEAR_CORE_MB_GPU_IndexBuffereUsage";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_Usage = MemoryBlock::Create(&mbCI);
	}
}

void IndexBuffer::Upload(miru::Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_IndexBufferUpload, m_IndexBuffer, { {0, 0, m_Count * sizeof(unsigned int)} });
		m_Upload = true;
	}
}
