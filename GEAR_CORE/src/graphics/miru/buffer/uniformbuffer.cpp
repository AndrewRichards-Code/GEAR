#include "uniformbuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

using namespace miru;
using namespace miru::crossplatform;

miru::Ref<miru::crossplatform::Context> UniformBuffer::s_Context = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> UniformBuffer::s_MB_CPU_Upload = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> UniformBuffer::s_MB_GPU_Usage = nullptr;

UniformBuffer::UniformBuffer(void* device, unsigned int size, unsigned int bindingIndex)
	:m_Device(device), m_Size(size), m_BindingIndex(bindingIndex)
{
	InitialiseMemory();

	m_UniformBufferUploadCI.debugName = "GEAR_CORE_UniformBufferUpload";
	m_UniformBufferUploadCI.device = m_Device;
	m_UniformBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_UniformBufferUploadCI.size = m_Size;
	m_UniformBufferUploadCI.data = nullptr;
	m_UniformBufferUploadCI.pMemoryBlock = s_MB_CPU_Upload;
	m_UniformBufferUpload = Buffer::Create(&m_UniformBufferUploadCI);

	m_UniformBufferCI.debugName = "GEAR_CORE_UniformBufferUsage";
	m_UniformBufferCI.device = m_Device;
	m_UniformBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::UNIFORM;
	m_UniformBufferCI.size = m_Size;
	m_UniformBufferCI.data = nullptr;
	m_UniformBufferCI.pMemoryBlock = s_MB_GPU_Usage;
	m_UniformBuffer = Buffer::Create(&m_UniformBufferCI);

	m_UniformBufferViewCI.debugName = "GEAR_CORE_UniformBufferViewUsage";
	m_UniformBufferViewCI.device = m_Device;
	m_UniformBufferViewCI.type = BufferView::Type::UNIFORM;
	m_UniformBufferViewCI.pBuffer = m_UniformBuffer;
	m_UniformBufferViewCI.offset = 0;
	m_UniformBufferViewCI.size = m_Size;
	m_UniformBufferViewCI.stride = 0;
	m_UniformBufferView = BufferView::Create(&m_UniformBufferViewCI);
}

UniformBuffer::~UniformBuffer()
{
}

void UniformBuffer::InitialiseMemory()
{
	MemoryBlock::CreateInfo mbCI;
	if (!s_MB_CPU_Upload)
	{
		mbCI.debugName = "GEAR_CORE_MB_CPU_UniformBufferUpload";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_32MB;
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_Upload = MemoryBlock::Create(&mbCI);
	}
	if (!s_MB_GPU_Usage)
	{
		mbCI.debugName = "GEAR_CORE_MB_GPU_UniformBuffereUsage";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_32MB;
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_Usage = MemoryBlock::Create(&mbCI);
	}
}

void UniformBuffer::SubmitData(const void* data, unsigned int size) const
{
	s_MB_CPU_Upload->SubmitData(m_UniformBufferUpload->GetResource(), (size_t)size, (void*)data);
}

void UniformBuffer::Upload(miru::Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_UniformBufferUpload, m_UniformBuffer, { {0, 0, m_Size} });
		m_Upload = true;
	}
}