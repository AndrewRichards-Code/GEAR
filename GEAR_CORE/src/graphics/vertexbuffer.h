#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
	class Vertexbuffer
	{
	public: 
		struct CreateInfo
		{
			const char*						debugName;
			void*							device;
			void*							data;
			size_t							size;
			miru::crossplatform::VertexType type;
		};
	
	private:
		miru::Ref<miru::crossplatform::Buffer> m_VertexBuffer, m_VertexBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_VertexBufferCI, m_VertexBufferUploadCI;
		
		miru::Ref<miru::crossplatform::BufferView> m_VertexBufferView;
		miru::crossplatform::BufferView::CreateInfo m_VertexBufferViewCI;

		std::string m_DebugName_VBUpload;
		std::string m_DebugName_VB;
		std::string m_DebugName_VBV;

		CreateInfo m_CI;
		bool m_Upload = false;

	public:
		Vertexbuffer(CreateInfo* pCreateInfo);
		~Vertexbuffer();

		void Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);
		inline const miru::Ref<miru::crossplatform::BufferView>& GetVertexBufferView() { return m_VertexBufferView; };

		static size_t GetVertexTypeSize(miru::crossplatform::VertexType type);
	};
}
}