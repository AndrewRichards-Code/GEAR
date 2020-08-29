#pragma once

#include "gear_core_common.h"

namespace gear 
{
namespace graphics 
{
	class Vertexbuffer
	{
	public: 
		struct CreateInfo
		{
			std::string debugName;
			void*		device;
			void*		data;
			size_t		size;
			size_t		stride;
		};
	
	private:
		miru::Ref<miru::crossplatform::Buffer> m_VertexBuffer, m_VertexBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_VertexBufferCI, m_VertexBufferUploadCI;
		
		miru::Ref<miru::crossplatform::BufferView> m_VertexBufferView;
		miru::crossplatform::BufferView::CreateInfo m_VertexBufferViewCI;

		CreateInfo m_CI;
		bool m_Upload = false;

	public:
		Vertexbuffer(CreateInfo* pCreateInfo);
		~Vertexbuffer();

		const CreateInfo& GetCreateInfo() { return m_CI; }

		void Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false);
		inline const miru::Ref<miru::crossplatform::BufferView>& GetVertexBufferView() { return m_VertexBufferView; };
	};
}
}