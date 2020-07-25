#include "gear_core_common.h"
/*#include "Storagebuffer.h"
#include "Graphics/MemoryBlockManager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Storagebuffer::Storagebuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_ShaderStorageBufferUploadCI.debugName = "GEAR_CORE_ShaderStorageBufferUpload";
	m_ShaderStorageBufferUploadCI.device = m_CI.device;
	m_ShaderStorageBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC | Buffer::UsageBit::TRANSFER_DST;
	m_ShaderStorageBufferUploadCI.size = m_CI.size;
	m_ShaderStorageBufferUploadCI.data = m_CI.data;
	m_ShaderStorageBufferUploadCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::CPU);
	m_ShaderStorageBufferUpload = Buffer::Create(&m_ShaderStorageBufferUploadCI);

	m_ShaderStorageBufferCI.debugName = "GEAR_CORE_ShaderStorageBuffer";
	m_ShaderStorageBufferCI.device = m_CI.device;
	m_ShaderStorageBufferCI.usage = Buffer::UsageBit::TRANSFER_SRC | Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::STORAGE;
	m_ShaderStorageBufferCI.size = m_CI.size;
	m_ShaderStorageBufferCI.data = nullptr;
	m_ShaderStorageBufferCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::GPU);
	m_ShaderStorageBuffer = Buffer::Create(&m_ShaderStorageBufferCI);

	m_ShaderStorageBufferViewCI.debugName = "GEAR_CORE_ShaderStorageViewUsage";
	m_ShaderStorageBufferViewCI.device = m_CI.device;
	m_ShaderStorageBufferViewCI.type = BufferView::Type::UNIFORM;
	m_ShaderStorageBufferViewCI.pBuffer = m_ShaderStorageBuffer;
	m_ShaderStorageBufferViewCI.offset = 0;
	m_ShaderStorageBufferViewCI.size = m_CI.size;
	m_ShaderStorageBufferViewCI.stride = 0;
	m_ShaderStorageBufferView = BufferView::Create(&m_ShaderStorageBufferViewCI);
}

Storagebuffer::~Storagebuffer()
{
}

void Storagebuffer::SubmitData(const void* data, size_t size) const
{
	m_ShaderStorageBufferUploadCI.pMemoryBlock->SubmitData(m_ShaderStorageBufferUpload->GetResource(), (size_t)size, (void*)data);
}

void Storagebuffer::AccessData(void* data, size_t size) const
{
	m_ShaderStorageBufferUploadCI.pMemoryBlock->AccessData(m_ShaderStorageBufferUpload->GetResource(), size, data);
}

void Storagebuffer::Upload(const miru::Ref<CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex)
{
	cmdBuffer->CopyBuffer(cmdBufferIndex, m_ShaderStorageBufferUpload, m_ShaderStorageBuffer, { {0, 0, m_CI.size} });
}

void Storagebuffer::Download(const miru::Ref<CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex)
{
	cmdBuffer->CopyBuffer(cmdBufferIndex, m_ShaderStorageBuffer, m_ShaderStorageBufferUpload, { {0, 0, m_CI.size} });
}*/