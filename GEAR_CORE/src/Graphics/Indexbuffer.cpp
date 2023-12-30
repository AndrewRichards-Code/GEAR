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

	Buffer::CreateInfo indexBufferUploadCI;
	indexBufferUploadCI.debugName = "GEAR_CORE_IndexBufferUpload: " + m_CI.debugName;
	indexBufferUploadCI.device = m_CI.device;
	indexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	indexBufferUploadCI.size = m_CI.size;
	indexBufferUploadCI.data = m_CI.data;
	indexBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
	m_IndexBufferUpload = Buffer::Create(&indexBufferUploadCI);

	Buffer::CreateInfo indexBufferCI;
	indexBufferCI.debugName = "GEAR_CORE_IndexBuffer: " + m_CI.debugName;
	indexBufferCI.device = m_CI.device;
	indexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::INDEX_BIT;
	indexBufferCI.size = m_CI.size;
	indexBufferCI.data = nullptr;
	indexBufferCI.allocator = AllocatorManager::GetGPUAllocator();
	m_IndexBuffer = Buffer::Create(&indexBufferCI);

	BufferView::CreateInfo indexBufferViewUploadCI;
	indexBufferViewUploadCI.debugName = "GEAR_CORE_IndexBufferViewUpload: " + m_CI.debugName;
	indexBufferViewUploadCI.device = m_CI.device;
	indexBufferViewUploadCI.type = BufferView::Type::INDEX;
	indexBufferViewUploadCI.buffer = m_IndexBufferUpload;
	indexBufferViewUploadCI.offset = 0;
	indexBufferViewUploadCI.size = m_CI.size;
	indexBufferViewUploadCI.stride = m_CI.stride;
	m_IndexBufferViewUpload = BufferView::Create(&indexBufferViewUploadCI);

	BufferView::CreateInfo indexBufferViewCI;
	indexBufferViewCI.debugName = "GEAR_CORE_IndexBufferView: " + m_CI.debugName;
	indexBufferViewCI.device = m_CI.device;
	indexBufferViewCI.type = BufferView::Type::INDEX;
	indexBufferViewCI.buffer = m_IndexBuffer;
	indexBufferViewCI.offset = 0;
	indexBufferViewCI.size = m_CI.size;
	indexBufferViewCI.stride = m_CI.stride;
	m_IndexBufferView = BufferView::Create(&indexBufferViewCI);
}

Indexbuffer::~Indexbuffer()
{
}