#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
class IndexBuffer
{
private:
	void* m_Device;

	static miru::Ref<miru::crossplatform::Context> s_Context;
	static miru::Ref<miru::crossplatform::MemoryBlock> s_MB_CPU_Upload, s_MB_GPU_Usage;

	miru::Ref<miru::crossplatform::Buffer> m_IndexBuffer, m_IndexBufferUpload;
	miru::crossplatform::Buffer::CreateInfo m_IndexBufferCI, m_IndexBufferUploadCI;

	miru::Ref<miru::crossplatform::BufferView> m_IndexBufferView;
	miru::crossplatform::BufferView::CreateInfo m_IndexBufferViewCI;

	unsigned int m_Count;
	bool m_Upload = false;

public:
	IndexBuffer(void* device, const unsigned int* data, unsigned int count);
	~IndexBuffer();

	inline static void SetContext(miru::Ref<miru::crossplatform::Context> context) { s_Context = context; };
	void InitialiseMemory();

	void Upload(miru::Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);

	inline unsigned int GetCount() const { return m_Count; }

	inline miru::Ref<miru::crossplatform::BufferView> GetIndexBufferView() { return m_IndexBufferView; };
};
}
}