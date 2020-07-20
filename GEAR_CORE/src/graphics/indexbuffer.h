#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
class IndexBuffer
{
public:
	struct CreateInfo
	{
		const char* debugName;
		void*		device;
		void*		data;
		size_t		size;
		uint32_t	stride;
	};
private:
	miru::Ref<miru::crossplatform::Buffer> m_IndexBuffer, m_IndexBufferUpload;
	miru::crossplatform::Buffer::CreateInfo m_IndexBufferCI, m_IndexBufferUploadCI;

	miru::Ref<miru::crossplatform::BufferView> m_IndexBufferView;
	miru::crossplatform::BufferView::CreateInfo m_IndexBufferViewCI;

	std::string m_DebugName_IBUpload;
	std::string m_DebugName_IB;
	std::string m_DebugName_IBV;

	CreateInfo m_CI;
	uint32_t m_Count;
	bool m_Upload = false;

public:
	IndexBuffer(CreateInfo* pCreateInfo);
	~IndexBuffer();

	void Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);
	inline miru::Ref<miru::crossplatform::BufferView> GetIndexBufferView() { return m_IndexBufferView; };

	inline uint32_t GetCount() const { return m_Count; }
};
}
}