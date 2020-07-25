#include "gear_core_common.h"
/*#include "uniformbuffer.h"
#include "graphics/memoryblockmanager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

UniformBuffer::UniformBuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_UniformBufferUploadCI.debugName = "GEAR_CORE_UniformBufferUpload";
	m_UniformBufferUploadCI.device = m_CI.device;
	m_UniformBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_UniformBufferUploadCI.size = GetSize();
	m_UniformBufferUploadCI.data = m_CI.data;
	m_UniformBufferUploadCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::CPU);
	m_UniformBufferUpload = Buffer::Create(&m_UniformBufferUploadCI);

	m_UniformBufferCI.debugName = "GEAR_CORE_UniformBuffer";
	m_UniformBufferCI.device = m_CI.device;
	m_UniformBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::UNIFORM;
	m_UniformBufferCI.size = GetSize();
	m_UniformBufferCI.data = nullptr;
	m_UniformBufferCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::GPU);
	m_UniformBuffer = Buffer::Create(&m_UniformBufferCI);

	m_UniformBufferViewCI.debugName = "GEAR_CORE_UniformBufferViewUsage";
	m_UniformBufferViewCI.device = m_CI.device;
	m_UniformBufferViewCI.type = BufferView::Type::UNIFORM;
	m_UniformBufferViewCI.pBuffer = m_UniformBuffer;
	m_UniformBufferViewCI.offset = 0;
	m_UniformBufferViewCI.size = GetSize();
	m_UniformBufferViewCI.stride = 0;
	m_UniformBufferView = BufferView::Create(&m_UniformBufferViewCI);
}

UniformBuffer::UniformBuffer()
{
}

void UniformBuffer::SubmitData() const
{
	m_UniformBufferUploadCI.pMemoryBlock->SubmitData(m_UniformBufferUpload->GetResource(), GetSize(), (void*)this);
}

void UniformBuffer::Upload(const miru::Ref<CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_UniformBufferUpload, m_UniformBuffer, { {0, 0, GetSize()} });
		m_Upload = true;
	}
}*/