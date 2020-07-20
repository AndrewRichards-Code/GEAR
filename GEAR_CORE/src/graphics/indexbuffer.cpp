#include "gear_core_common.h"
#include "indexbuffer.h"
#include "graphics/memoryblockmanager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

IndexBuffer::IndexBuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	GEAR_ASSERT((m_CI.stride != 2 && m_CI.stride != 4), "ERROR: GEAR::GRAPHICS::IndexBuffer: Stride is not 2 or 4.");
	GEAR_ASSERT((m_CI.size % m_CI.stride), "ERROR: GEAR::GRAPHICS::IndexBuffer: Size is not a multiple of the stride.");
	m_Count = static_cast<uint32_t>(m_CI.size) / m_CI.stride;

	m_DebugName_IBUpload = std::string("GEAR_CORE_IndexBufferUpload: ") + m_CI.debugName;
	m_IndexBufferUploadCI.debugName = m_DebugName_IBUpload.c_str();
	m_IndexBufferUploadCI.device = m_CI.device;
	m_IndexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_IndexBufferUploadCI.size = m_CI.size;
	m_IndexBufferUploadCI.data = m_CI.data;
	m_IndexBufferUploadCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::CPU);
	m_IndexBufferUpload = Buffer::Create(&m_IndexBufferUploadCI);

	m_DebugName_IB = std::string("GEAR_CORE_IndexBuffer: ") + m_CI.debugName;
	m_IndexBufferCI.debugName = m_DebugName_IB.c_str();
	m_IndexBufferCI.device = m_CI.device;
	m_IndexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::INDEX;
	m_IndexBufferCI.size = m_CI.size;
	m_IndexBufferCI.data = nullptr;
	m_IndexBufferCI.pMemoryBlock =MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::GPU);
	m_IndexBuffer = Buffer::Create(&m_IndexBufferCI);

	m_DebugName_IBV = std::string("GEAR_CORE_VertexBufferView: ") + m_CI.debugName;
	m_IndexBufferViewCI.debugName = "GEAR_CORE_VertexBufferView";
	m_IndexBufferViewCI.device = m_CI.device;
	m_IndexBufferViewCI.type = BufferView::Type::INDEX;
	m_IndexBufferViewCI.pBuffer = m_IndexBuffer;
	m_IndexBufferViewCI.offset = 0;
	m_IndexBufferViewCI.size = m_CI.size;
	m_IndexBufferViewCI.stride = m_CI.stride;
	m_IndexBufferView = BufferView::Create(&m_IndexBufferViewCI);
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_IndexBufferUpload, m_IndexBuffer, { {0, 0, m_CI.size } });
		m_Upload = true;
	}
}
