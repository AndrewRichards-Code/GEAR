#pragma once
#include "gear_core_common.h"

namespace gear 
{
	namespace graphics
	{
		class GEAR_API Vertexbuffer
		{
		public:
			struct CreateInfo
			{
				std::string	debugName;
				void*		device;
				void*		data;
				size_t		size;
				size_t		stride;
			};

		private:
			miru::base::BufferRef m_VertexBuffer, m_VertexBufferUpload;
			miru::base::BufferViewRef m_VertexBufferView, m_VertexBufferViewUpload;

			CreateInfo m_CI;

		public:
			Vertexbuffer(CreateInfo* pCreateInfo);
			~Vertexbuffer();

			const CreateInfo& GetCreateInfo() { return m_CI; }

			inline const miru::base::BufferRef& GetCPUBuffer() { return m_VertexBufferUpload; };
			inline const miru::base::BufferRef& GetGPUBuffer() { return m_VertexBuffer; };
			inline const miru::base::BufferViewRef& GetCPUBufferView() { return m_VertexBufferViewUpload; };
			inline const miru::base::BufferViewRef& GetGPUBufferView() { return m_VertexBufferView; };
		};
	}
}