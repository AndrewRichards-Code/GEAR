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

	Buffer::CreateInfo vertexBufferUploadCI;
	vertexBufferUploadCI.debugName = "GEAR_CORE_VertexBufferUpload: " + m_CI.debugName;
	vertexBufferUploadCI.device = m_CI.device;
	vertexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC_BIT;
	vertexBufferUploadCI.size = m_CI.size;
	vertexBufferUploadCI.data = m_CI.data;
	vertexBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
	m_VertexBufferUpload = Buffer::Create(&vertexBufferUploadCI);

	Buffer::CreateInfo vertexBufferCI;
	vertexBufferCI.debugName = "GEAR_CORE_VertexBuffer: " + m_CI.debugName;
	vertexBufferCI.device = m_CI.device;
	vertexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST_BIT | Buffer::UsageBit::VERTEX_BIT;
	vertexBufferCI.size = m_CI.size;
	vertexBufferCI.data = nullptr;
	vertexBufferCI.allocator = AllocatorManager::GetGPUAllocator();
	m_VertexBuffer = Buffer::Create(&vertexBufferCI);

	BufferView::CreateInfo vertexBufferViewUploadCI;
	vertexBufferViewUploadCI.debugName = "GEAR_CORE_VertexBufferViewUpload: " + m_CI.debugName;
	vertexBufferViewUploadCI.device = m_CI.device;
	vertexBufferViewUploadCI.type = BufferView::Type::VERTEX;
	vertexBufferViewUploadCI.buffer = m_VertexBufferUpload;
	vertexBufferViewUploadCI.offset = 0;
	vertexBufferViewUploadCI.size = m_CI.size;
	vertexBufferViewUploadCI.stride = m_CI.stride;
	m_VertexBufferViewUpload = BufferView::Create(&vertexBufferViewUploadCI);

	BufferView::CreateInfo vertexBufferViewCI;
	vertexBufferViewCI.debugName = "GEAR_CORE_VertexBufferView: " + m_CI.debugName;
	vertexBufferViewCI.device = m_CI.device;
	vertexBufferViewCI.type = BufferView::Type::VERTEX;
	vertexBufferViewCI.buffer = m_VertexBuffer;
	vertexBufferViewCI.offset = 0;
	vertexBufferViewCI.size = m_CI.size;
	vertexBufferViewCI.stride = m_CI.stride;
	m_VertexBufferView = BufferView::Create(&vertexBufferViewCI);
}

Vertexbuffer::~Vertexbuffer()
{
}