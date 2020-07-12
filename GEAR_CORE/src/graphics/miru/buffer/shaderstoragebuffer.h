#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
	class ShaderStorageBuffer
	{
	public:
		struct CreateInfo
		{
			void*	device;
			void*	data;
			size_t	size;
		};

	private:
		static miru::Ref<miru::crossplatform::Context> s_Context;
		static miru::Ref<miru::crossplatform::MemoryBlock> s_MB_CPU_Upload, s_MB_GPU_Usage;

		miru::Ref<miru::crossplatform::Buffer> m_ShaderStorageBuffer, m_ShaderStorageBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_ShaderStorageBufferCI, m_ShaderStorageBufferUploadCI;

		miru::Ref<miru::crossplatform::BufferView> m_ShaderStorageBufferView;
		miru::crossplatform::BufferView::CreateInfo m_ShaderStorageBufferViewCI;

		CreateInfo m_CI;

	public:
		ShaderStorageBuffer(CreateInfo* pCreateInfo);
		~ShaderStorageBuffer();

		inline static void SetContext(miru::Ref<miru::crossplatform::Context> context) { s_Context = context; };
		void InitialiseMemory();

		void SubmitData(const void* data, size_t  size) const;
		void AccessData(void* data, size_t size) const;
		void Upload(miru::crossplatform::CommandBuffer& cmdBuffer, uint32_t cmdBufferIndex = 0);
		void Download(miru::crossplatform::CommandBuffer& cmdBuffer, uint32_t cmdBufferIndex = 0);

		inline miru::Ref<miru::crossplatform::Buffer> GetBuffer() const { return m_ShaderStorageBuffer; };
		inline miru::Ref<miru::crossplatform::BufferView> GetBufferView() const { return m_ShaderStorageBufferView; };

		inline size_t GetSize() { return m_CI.size; }
	};
}
}