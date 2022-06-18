#include "gear_core_common.h"
#include "Graphics/Vertexbuffer.h"
#include "Graphics/AllocatorManager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace base;

Vertexbuffer::Vertexbuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_VertexBufferUploadCI.debugName = "GEAR_CORE_VertexBufferUpload: " + m_CI.debugName;
	m_VertexBufferUploadCI.device = m_CI.device;
	m_VertexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	m_VertexBufferUploadCI.size = m_CI.size;
	m_VertexBufferUploadCI.data = m_CI.data;
	m_VertexBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
	m_VertexBufferUpload = Buffer::Create(&m_VertexBufferUploadCI);

	m_VertexBufferCI.debugName = "GEAR_CORE_VertexBuffer: " + m_CI.debugName;
	m_VertexBufferCI.device = m_CI.device;
	m_VertexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::VERTEX_BIT;
	m_VertexBufferCI.size = m_CI.size;
	m_VertexBufferCI.data = nullptr;
	m_VertexBufferCI.allocator = AllocatorManager::GetGPUAllocator();
	m_VertexBuffer = Buffer::Create(&m_VertexBufferCI);

	m_VertexBufferViewUploadCI.debugName = "GEAR_CORE_VertexBufferViewUpload: " + m_CI.debugName;
	m_VertexBufferViewUploadCI.device = m_CI.device;
	m_VertexBufferViewUploadCI.type = BufferView::Type::VERTEX;
	m_VertexBufferViewUploadCI.buffer = m_VertexBufferUpload;
	m_VertexBufferViewUploadCI.offset = 0;
	m_VertexBufferViewUploadCI.size = m_CI.size;
	m_VertexBufferViewUploadCI.stride = m_CI.stride;
	m_VertexBufferViewUpload = BufferView::Create(&m_VertexBufferViewUploadCI);

	m_VertexBufferViewCI.debugName = "GEAR_CORE_VertexBufferView: " + m_CI.debugName;
	m_VertexBufferViewCI.device = m_CI.device;
	m_VertexBufferViewCI.type = BufferView::Type::VERTEX;
	m_VertexBufferViewCI.buffer = m_VertexBuffer;
	m_VertexBufferViewCI.offset = 0;
	m_VertexBufferViewCI.size = m_CI.size;
	m_VertexBufferViewCI.stride = m_CI.stride;
	m_VertexBufferView = BufferView::Create(&m_VertexBufferViewCI);
}

Vertexbuffer::~Vertexbuffer()
{
}