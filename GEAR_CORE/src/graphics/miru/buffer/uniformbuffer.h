#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
	class UniformBuffer
	{
	private:
		void* m_Device;

		static miru::Ref<miru::crossplatform::Context> s_Context;
		static miru::Ref<miru::crossplatform::MemoryBlock> s_MB_CPU_Upload, s_MB_GPU_Usage;

		miru::Ref<miru::crossplatform::Buffer> m_UniformBuffer, m_UniformBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_UniformBufferCI, m_UniformBufferUploadCI;

		miru::Ref<miru::crossplatform::BufferView> m_UniformBufferView;
		miru::crossplatform::BufferView::CreateInfo m_UniformBufferViewCI;

		unsigned int m_Size;
		unsigned int m_BindingIndex;
		bool m_Upload = false;

	public:
		UniformBuffer(void* device, unsigned int size, unsigned int bindingIndex);
		~UniformBuffer();

		inline static void SetContext(miru::Ref<miru::crossplatform::Context> context) { s_Context = context; };
		void InitialiseMemory();
		
		void SubmitData(const void* data, unsigned int size) const;
		void Upload(miru::Ref<miru::crossplatform::CommandBuffer> cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);

		inline miru::Ref<miru::crossplatform::Buffer> GetBuffer() const { return m_UniformBuffer; };
		inline miru::Ref<miru::crossplatform::BufferView> GetBufferView() const { return m_UniformBufferView; };

		inline unsigned int GetSize() { return m_Size; }
		inline unsigned int GetBindingIndex() { return m_BindingIndex; }
	};
}
}