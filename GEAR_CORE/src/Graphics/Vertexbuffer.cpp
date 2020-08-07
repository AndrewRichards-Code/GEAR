#include "gear_core_common.h"
#include "Vertexbuffer.h"
#include "Graphics/MemoryBlockManager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Vertexbuffer::Vertexbuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	m_DebugName_VBUpload = "GEAR_CORE_VertexBufferUpload: " + m_CI.debugName;
	m_VertexBufferUploadCI.debugName = m_DebugName_VBUpload.c_str();
	m_VertexBufferUploadCI.device = m_CI.device;
	m_VertexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_VertexBufferUploadCI.size = m_CI.size;
	m_VertexBufferUploadCI.data = m_CI.data;
	m_VertexBufferUploadCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::CPU);
	m_VertexBufferUpload = Buffer::Create(&m_VertexBufferUploadCI);

	m_DebugName_VB = "GEAR_CORE_VertexBuffer: " + m_CI.debugName;
	m_VertexBufferCI.debugName = m_DebugName_VB.c_str();
	m_VertexBufferCI.device = m_CI.device;
	m_VertexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::VERTEX;
	m_VertexBufferCI.size = m_CI.size;
	m_VertexBufferCI.data = nullptr;
	m_VertexBufferCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::GPU);
	m_VertexBuffer = Buffer::Create(&m_VertexBufferCI);

	m_DebugName_VBV = "GEAR_CORE_VertexBufferViewUsage: " + m_CI.debugName;
	m_VertexBufferViewCI.debugName = m_DebugName_VBV.c_str();
	m_VertexBufferViewCI.device = m_CI.device;
	m_VertexBufferViewCI.type = BufferView::Type::VERTEX;
	m_VertexBufferViewCI.pBuffer = m_VertexBuffer;
	m_VertexBufferViewCI.offset = 0;
	m_VertexBufferViewCI.size = m_CI.size;
	m_VertexBufferViewCI.stride = m_CI.stride;
	m_VertexBufferView = BufferView::Create(&m_VertexBufferViewCI);
}

Vertexbuffer::~Vertexbuffer()
{
}

void Vertexbuffer::Upload(const miru::Ref<CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_VertexBufferUpload, m_VertexBuffer, { {0, 0, m_CI.size} });
		m_Upload = true;
	}
}