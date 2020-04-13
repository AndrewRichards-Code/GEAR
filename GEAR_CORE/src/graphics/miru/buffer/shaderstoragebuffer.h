#pragma once

#include "gear_core_common.h"

namespace GEAR {
namespace GRAPHICS {
	class ShaderStorageBuffer
	{
	private:
		void* m_Device;

		static miru::Ref<miru::crossplatform::Context> s_Context;
		static miru::Ref<miru::crossplatform::MemoryBlock> s_MB_CPU_Upload, s_MB_GPU_Usage;

		miru::Ref<miru::crossplatform::Buffer> m_ShaderStorageBuffer, m_ShaderStorageBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_ShaderStorageBufferCI, m_ShaderStorageBufferUploadCI;

		unsigned int m_Size;
		unsigned int m_BindingIndex;

	public:
		ShaderStorageBuffer(unsigned int size, unsigned int bindingIndex);
		~ShaderStorageBuffer();

		inline static void SetContext(miru::Ref<miru::crossplatform::Context> context) { s_Context = context; };
		void InitialiseMemory();

		void SubmitData(const void* data, unsigned int size, unsigned int offset) const;
		void Upload(miru::crossplatform::CommandBuffer& cmdBuffer, uint32_t cmdBufferIndex = 0);
		void Download(miru::crossplatform::CommandBuffer& cmdBuffer, uint32_t cmdBufferIndex = 0);
		void AccessData(void* data, unsigned int size, unsigned int offset) const;

		inline unsigned int GetSize() { return m_Size; }
		inline unsigned int GetBindingIndex() { return m_BindingIndex; }
	};
}
}