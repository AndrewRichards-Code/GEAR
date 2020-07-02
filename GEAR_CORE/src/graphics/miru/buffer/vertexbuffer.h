#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
	class VertexBuffer
	{
	private:
		void* m_Device;

		static miru::Ref<miru::crossplatform::Context> s_Context;
		static miru::Ref<miru::crossplatform::MemoryBlock> s_MB_CPU_Upload, s_MB_GPU_Usage;

		miru::Ref<miru::crossplatform::Buffer> m_VertexBuffer, m_VertexBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_VertexBufferCI, m_VertexBufferUploadCI;
		
		miru::Ref<miru::crossplatform::BufferView> m_VertexBufferView;
		miru::crossplatform::BufferView::CreateInfo m_VertexBufferViewCI;

		unsigned int m_Count;
		unsigned int m_ComponentCount;
		bool m_Upload = false;

	public:
		VertexBuffer(void* device, const float* data, unsigned int count, unsigned int componentCount);
		~VertexBuffer();

		inline static void SetContext(miru::Ref<miru::crossplatform::Context> context) { s_Context = context; };
		void InitialiseMemory();

		void Upload(miru::Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);

		inline unsigned int GetCount() { return m_Count; }
		inline unsigned int GetComponentCount() { return m_ComponentCount; }

		inline miru::Ref<miru::crossplatform::BufferView> GetVertexBufferView() { return m_VertexBufferView; };
	};
}
}