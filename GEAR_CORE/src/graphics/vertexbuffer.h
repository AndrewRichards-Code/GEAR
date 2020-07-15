#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
	class VertexBuffer
	{
	public: 
		struct CreateInfo
		{
			void*							device;
			void*							data;
			size_t							size;
			miru::crossplatform::VertexType type;
		};
	
	private:
		static miru::Ref<miru::crossplatform::Context> s_Context;
		static miru::Ref<miru::crossplatform::MemoryBlock> s_MB_CPU_Upload, s_MB_GPU_Usage;

		miru::Ref<miru::crossplatform::Buffer> m_VertexBuffer, m_VertexBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_VertexBufferCI, m_VertexBufferUploadCI;
		
		miru::Ref<miru::crossplatform::BufferView> m_VertexBufferView;
		miru::crossplatform::BufferView::CreateInfo m_VertexBufferViewCI;

		CreateInfo m_CI;
		bool m_Upload = false;

	public:
		VertexBuffer(CreateInfo* pCreateInfo);
		~VertexBuffer();

		inline static void SetContext(const miru::Ref<miru::crossplatform::Context>& context) { s_Context = context; };
		void InitialiseMemory();

		void Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);
		inline const miru::Ref<miru::crossplatform::BufferView>& GetVertexBufferView() { return m_VertexBufferView; };

		static size_t GetVertexTypeSize(miru::crossplatform::VertexType type);
	};
}
}