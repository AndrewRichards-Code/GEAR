#include "gear_core_common.h"
#include "Indexbuffer.h"
#include "Graphics/AllocatorManager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Indexbuffer::Indexbuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if (m_CI.stride != 2 && m_CI.stride != 4)
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Stride is not 2 or 4.");
	}
	if (m_CI.size % m_CI.stride)
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Size is not a multiple of the stride.");
	}	
	m_Count = static_cast<uint32_t>(m_CI.size / m_CI.stride);

	m_IndexBufferUploadCI.debugName = "GEAR_CORE_IndexBufferUpload: " + m_CI.debugName;
	m_IndexBufferUploadCI.device = m_CI.device;
	m_IndexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	m_IndexBufferUploadCI.size = m_CI.size;
	m_IndexBufferUploadCI.data = m_CI.data;
	m_IndexBufferUploadCI.pAllocator = AllocatorManager::GetCPUAllocator();
	m_IndexBufferUpload = Buffer::Create(&m_IndexBufferUploadCI);

	m_IndexBufferCI.debugName = "GEAR_CORE_IndexBuffer: " + m_CI.debugName;
	m_IndexBufferCI.device = m_CI.device;
	m_IndexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT;
	m_IndexBufferCI.size = m_CI.size;
	m_IndexBufferCI.data = nullptr;
	m_IndexBufferCI.pAllocator =AllocatorManager::GetGPUAllocator();
	m_IndexBuffer = Buffer::Create(&m_IndexBufferCI);

	m_IndexBufferViewCI.debugName = "GEAR_CORE_VertexBufferView: " + m_CI.debugName;
	m_IndexBufferViewCI.device = m_CI.device;
	m_IndexBufferViewCI.type = BufferView::Type::INDEX;
	m_IndexBufferViewCI.pBuffer = m_IndexBuffer;
	m_IndexBufferViewCI.offset = 0;
	m_IndexBufferViewCI.size = m_CI.size;
	m_IndexBufferViewCI.stride = m_CI.stride;
	m_IndexBufferView = BufferView::Create(&m_IndexBufferViewCI);
}

Indexbuffer::~Indexbuffer()
{
}

void Indexbuffer::Upload(const Ref<CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_IndexBufferUpload, m_IndexBuffer, { {0, 0, m_CI.size } });
		m_Upload = true;
	}
}
