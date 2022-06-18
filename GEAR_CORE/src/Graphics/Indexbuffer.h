#pragma once

#include "gear_core_common.h"

namespace gear
{
	namespace graphics
	{
		class GEAR_API Indexbuffer
		{
		public:
			struct CreateInfo
			{
				std::string debugName;
				void* device;
				void* data;
				size_t		size;
				size_t		stride;
			};

		private:
			miru::base::BufferRef m_IndexBuffer, m_IndexBufferUpload;
			miru::base::Buffer::CreateInfo m_IndexBufferCI, m_IndexBufferUploadCI;

			miru::base::BufferViewRef m_IndexBufferView, m_IndexBufferViewUpload;
			miru::base::BufferView::CreateInfo m_IndexBufferViewCI, m_IndexBufferViewUploadCI;

			CreateInfo m_CI;

			uint32_t m_Count;

		public:
			Indexbuffer(CreateInfo* pCreateInfo);
			~Indexbuffer();

			const CreateInfo& GetCreateInfo() { return m_CI; }

			inline miru::base::BufferRef GetCPUBuffer() { return m_IndexBufferUpload; };
			inline miru::base::BufferRef GetGPUBuffer() { return m_IndexBuffer; };
			inline miru::base::BufferViewRef GetCPUBufferView() { return m_IndexBufferViewUpload; };
			inline miru::base::BufferViewRef GetGPUBufferView() { return m_IndexBufferView; };

			inline uint32_t GetCount() const { return m_Count; }
		};
	}
}