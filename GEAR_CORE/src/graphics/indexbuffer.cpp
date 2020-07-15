#include "gear_core_common.h"
#include "indexbuffer.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

miru::Ref<miru::crossplatform::Context> IndexBuffer::s_Context = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> IndexBuffer::s_MB_CPU_Upload = nullptr;
miru::Ref<miru::crossplatform::MemoryBlock> IndexBuffer::s_MB_GPU_Usage = nullptr;

IndexBuffer::IndexBuffer(CreateInfo* pCreateInfo)
{
	m_CI = *pCreateInfo;

	InitialiseMemory();

	GEAR_ASSERT((m_CI.stride != 2 && m_CI.stride != 4), "ERROR: GEAR::GRAPHICS::IndexBuffer: Stride is not 2 or 4.");
	GEAR_ASSERT((m_CI.size % m_CI.stride), "ERROR: GEAR::GRAPHICS::IndexBuffer: Size is not a multiple of the stride.");
	m_Count = static_cast<uint32_t>(m_CI.size) / m_CI.stride;

	m_IndexBufferUploadCI.debugName = "GEAR_CORE_IndexBufferUpload";
	m_IndexBufferUploadCI.device = m_CI.device;
	m_IndexBufferUploadCI.usage = Buffer::UsageBit::TRANSFER_SRC;
	m_IndexBufferUploadCI.size = m_CI.size;
	m_IndexBufferUploadCI.data = m_CI.data;
	m_IndexBufferUploadCI.pMemoryBlock = s_MB_CPU_Upload;
	m_IndexBufferUpload = Buffer::Create(&m_IndexBufferUploadCI);

	m_IndexBufferCI.debugName = "GEAR_CORE_IndexBuffer";
	m_IndexBufferCI.device = m_CI.device;
	m_IndexBufferCI.usage = Buffer::UsageBit::TRANSFER_DST | Buffer::UsageBit::INDEX;
	m_IndexBufferCI.size = m_CI.size;
	m_IndexBufferCI.data = nullptr;
	m_IndexBufferCI.pMemoryBlock = s_MB_GPU_Usage;
	m_IndexBuffer = Buffer::Create(&m_IndexBufferCI);

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

void IndexBuffer::InitialiseMemory()
{
	MemoryBlock::CreateInfo mbCI;
	if (!s_MB_CPU_Upload)
	{
		mbCI.debugName = "GEAR_CORE_MB_CPU_IndexBufferUpload";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_Upload = MemoryBlock::Create(&mbCI);
	}
	if (!s_MB_GPU_Usage)
	{
		mbCI.debugName = "GEAR_CORE_MB_GPU_IndexBuffer";
		mbCI.pContext = s_Context;
		mbCI.blockSize = MemoryBlock::BlockSize::BLOCK_SIZE_128MB;
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_Usage = MemoryBlock::Create(&mbCI);
	}
}

void IndexBuffer::Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex, bool force)
{
	if (!m_Upload || force)
	{
		cmdBuffer->CopyBuffer(cmdBufferIndex, m_IndexBufferUpload, m_IndexBuffer, { {0, 0, m_CI.size } });
		m_Upload = true;
	}
}
