#include "gear_core_common.h"
#include "Graphics/Indexbuffer.h"
#include "Graphics/AllocatorManager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace base;

Indexbuffer::Indexbuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	if (m_CI.stride != 2 && m_CI.stride != 4)
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Stride is not 2 or 4.");
	}
	if (m_CI.size % m_CI.stride)
	{
		GEAR_FATAL(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "Size is not a multiple of the stride.");
	}	
	m_Count = static_cast<uint32_t>(m_CI.size / m_CI.stride);

	m_IndexBufferUploadCI.debugName = "GEAR_CORE_IndexBufferUpload: " + m_CI.debugName;
	m_IndexBufferUploadCI.device = m_CI.device;
	m_IndexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	m_IndexBufferUploadCI.size = m_CI.size;
	m_IndexBufferUploadCI.data = m_CI.data;
	m_IndexBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
	m_IndexBufferUpload = Buffer::Create(&m_IndexBufferUploadCI);

	m_IndexBufferCI.debugName = "GEAR_CORE_IndexBuffer: " + m_CI.debugName;
	m_IndexBufferCI.device = m_CI.device;
	m_IndexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT;
	m_IndexBufferCI.size = m_CI.size;
	m_IndexBufferCI.data = nullptr;
	m_IndexBufferCI.allocator = AllocatorManager::GetGPUAllocator();
	m_IndexBuffer = Buffer::Create(&m_IndexBufferCI);

	m_IndexBufferViewUploadCI.debugName = "GEAR_CORE_IndexBufferViewUpload: " + m_CI.debugName;
	m_IndexBufferViewUploadCI.device = m_CI.device;
	m_IndexBufferViewUploadCI.type = BufferView::Type::INDEX;
	m_IndexBufferViewUploadCI.buffer = m_IndexBufferUpload;
	m_IndexBufferViewUploadCI.offset = 0;
	m_IndexBufferViewUploadCI.size = m_CI.size;
	m_IndexBufferViewUploadCI.stride = m_CI.stride;
	m_IndexBufferViewUpload = BufferView::Create(&m_IndexBufferViewUploadCI);

	m_IndexBufferViewCI.debugName = "GEAR_CORE_IndexBufferView: " + m_CI.debugName;
	m_IndexBufferViewCI.device = m_CI.device;
	m_IndexBufferViewCI.type = BufferView::Type::INDEX;
	m_IndexBufferViewCI.buffer = m_IndexBuffer;
	m_IndexBufferViewCI.offset = 0;
	m_IndexBufferViewCI.size = m_CI.size;
	m_IndexBufferViewCI.stride = m_CI.stride;
	m_IndexBufferView = BufferView::Create(&m_IndexBufferViewCI);
}

Indexbuffer::~Indexbuffer()
{
}