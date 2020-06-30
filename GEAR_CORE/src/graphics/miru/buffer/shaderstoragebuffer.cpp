#include "shaderstoragebuffer.h"

using namespace GEAR;
using namespace GRAPHICS;

using namespace miru;
using namespace miru::crossplatform;

miru::Ref<miru::crossplatform::Context> ShaderStorageBuffer::s_Context = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> ShaderStorageBuffer::s_MB_CPU_Upload = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> ShaderStorageBuffer::s_MB_GPU_Usage = nullptr;

ShaderStorageBuffer::ShaderStorageBuffer(unsigned int size, unsigned int bindingIndex)
	:m_Size(size), m_BindingIndex(bindingIndex)
{
	InitialiseMemory();

	m_ShaderStorageBufferUploadCI.debugName = "GEAR_CORE_ShaderStorageBufferUpload";
	m_ShaderStorageBufferUploadCI.device = m_Device;
	m_ShaderStorageBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC | Buffer::UsageBit::TRANSFER_DST;
	m_ShaderStorageBufferUploadCI.size = m_Size;
	m_ShaderStorageBufferUploadCI.data = nullptr;
	m_ShaderStorageBufferUploadCI.pMemoryBlock = s_MB_CPU_Upload;
	m_ShaderStorageBufferUpload = Buffer::Create(&m_ShaderStorageBufferUploadCI);

	m_ShaderStorageBufferCI.debugName = "GEAR_CORE_ShaderStorageBuffer";
	m_ShaderStorageBufferCI.device = m_Device;
	m_ShaderStorageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC | Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::STORAGE;
	m_ShaderStorageBufferCI.size = m_Size;
	m_ShaderStorageBufferCI.data = nullptr;
	m_ShaderStorageBufferCI.pMemoryBlock = s_MB_GPU_Usage;
	m_ShaderStorageBuffer = Buffer::Create(&m_ShaderStorageBufferCI);
}

ShaderStorageBuffer::~ShaderStorageBuffer()
{
}

void ShaderStorageBuffer::InitialiseMemory()
{
	MemoryBlock::CreateInfo mbCI;
	if (!s_MB_CPU_Upload)
	{
		mbCI.debugName = "GEAR_CORE_MB_CPU_ShaderStorageBufferUpload";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_64MB;
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_Upload = MemoryBlock::Create(&mbCI);
	}
	if (!s_MB_GPU_Usage)
	{
		mbCI.debugName = "GEAR_CORE_MB_GPU_ShaderStorageBuffer";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_64MB;
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_Usage = MemoryBlock::Create(&mbCI);
	}
}

void ShaderStorageBuffer::SubmitData(const void* data, unsigned int size) const
{
	s_MB_CPU_Upload->SubmitData(m_ShaderStorageBufferUpload->GetResource(), (size_t)size, (void*)data);
}

void ShaderStorageBuffer::Upload(CommandBuffer& cmdBuffer, uint32_t cmdBufferIndex)
{
	cmdBuffer.CopyBuffer(cmdBufferIndex, m_ShaderStorageBufferUpload, m_ShaderStorageBuffer, { {0, 0, m_Size} });
}

void ShaderStorageBuffer::Download(CommandBuffer& cmdBuffer, uint32_t cmdBufferIndex)
{
	cmdBuffer.CopyBuffer(cmdBufferIndex, m_ShaderStorageBuffer, m_ShaderStorageBufferUpload, { {0, 0, m_Size} });
}

void ShaderStorageBuffer::AccessData(void* data, unsigned int size) const
{
	s_MB_CPU_Upload->AccessData(m_ShaderStorageBufferUpload->GetResource(), (size_t)size, (void*)data);
}