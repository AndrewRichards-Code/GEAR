#pragma once
#include "gear_core_common.h"
#include "AllocatorManager.h"

namespace gear
{
	namespace graphics
	{
		class GEAR_API BaseStoragebuffer
		{
		public:
			virtual ~BaseStoragebuffer() = default;

			virtual const miru::base::BufferRef& GetCPUBuffer() const = 0;
			virtual const miru::base::BufferRef& GetGPUBuffer() const = 0;
			virtual const miru::base::BufferViewRef& GetCPUBufferView() const = 0;
			virtual const miru::base::BufferViewRef& GetGPUBufferView() const = 0;
		};

		template<typename T>
		class GEAR_API Storagebuffer : public T, public BaseStoragebuffer
		{
		public:
			struct CreateInfo
			{
				std::string debugName;
				void* device;
				void* data;
			};

		private:
			miru::base::BufferRef m_ShaderStorageBuffer, m_ShaderStorageBufferUpload;
			miru::base::Buffer::CreateInfo m_ShaderStorageBufferCI, m_ShaderStorageBufferUploadCI;

			miru::base::BufferViewRef m_ShaderStorageBufferView, m_ShaderStorageBufferViewUpload;
			miru::base::BufferView::CreateInfo m_ShaderStorageBufferViewCI, m_ShaderStorageBufferViewUploadCI;

			CreateInfo m_CI;

		public:
			Storagebuffer(CreateInfo* pCreateInfo)
			{
				m_CI = *pCreateInfo;

				m_ShaderStorageBufferUploadCI.debugName = "GEAR_CORE_ShaderStorageBufferUpload: " + m_CI.debugName;
				m_ShaderStorageBufferUploadCI.device = m_CI.device;
				m_ShaderStorageBufferUploadCI.usage = miru::base::Buffer::UsageBit::TRANSFER_SRC_BIT | miru::base::Buffer::UsageBit::TRANSFER_DST_BIT;
				m_ShaderStorageBufferUploadCI.size = GetSize();
				m_ShaderStorageBufferUploadCI.data = m_CI.data;
				m_ShaderStorageBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
				m_ShaderStorageBufferUpload = miru::base::Buffer::Create(&m_ShaderStorageBufferUploadCI);

				m_ShaderStorageBufferCI.debugName = "GEAR_CORE_ShaderStorageBuffer: " + m_CI.debugName;
				m_ShaderStorageBufferCI.device = m_CI.device;
				m_ShaderStorageBufferCI.usage = miru::base::Buffer::UsageBit::TRANSFER_SRC_BIT | miru::base::Buffer::UsageBit::TRANSFER_DST_BIT | miru::base::Buffer::UsageBit::STORAGE_BIT;
				m_ShaderStorageBufferCI.size = GetSize();
				m_ShaderStorageBufferCI.data = nullptr;
				m_ShaderStorageBufferCI.allocator = AllocatorManager::GetGPUAllocator();
				m_ShaderStorageBuffer = miru::base::Buffer::Create(&m_ShaderStorageBufferCI);

				m_ShaderStorageBufferViewUploadCI.debugName = "GEAR_CORE_ShaderStorageViewUpload: " + m_CI.debugName;
				m_ShaderStorageBufferViewUploadCI.device = m_CI.device;
				m_ShaderStorageBufferViewUploadCI.type = miru::base::BufferView::Type::UNIFORM;
				m_ShaderStorageBufferViewUploadCI.buffer = m_ShaderStorageBufferUpload;
				m_ShaderStorageBufferViewUploadCI.offset = 0;
				m_ShaderStorageBufferViewUploadCI.size = GetSize();
				m_ShaderStorageBufferViewUploadCI.stride = 0;
				m_ShaderStorageBufferViewUpload = miru::base::BufferView::Create(&m_ShaderStorageBufferViewUploadCI);

				m_ShaderStorageBufferViewCI.debugName = "GEAR_CORE_ShaderStorageView: " + m_CI.debugName;
				m_ShaderStorageBufferViewCI.device = m_CI.device;
				m_ShaderStorageBufferViewCI.type = miru::base::BufferView::Type::UNIFORM;
				m_ShaderStorageBufferViewCI.buffer = m_ShaderStorageBuffer;
				m_ShaderStorageBufferViewCI.offset = 0;
				m_ShaderStorageBufferViewCI.size = GetSize();
				m_ShaderStorageBufferViewCI.stride = 0;
				m_ShaderStorageBufferView = miru::base::BufferView::Create(&m_ShaderStorageBufferViewCI);
			}
			~Storagebuffer() {}

			const CreateInfo& GetCreateInfo() { return m_CI; }

			void SubmitData(const void* data, size_t  size) const
			{
				m_ShaderStorageBufferUploadCI.pAllocator->SubmitData(m_ShaderStorageBufferUpload->GetAllocation(), (size_t)size, (void*)data);
			}
			void AccessData(void* data, size_t size) const
			{
				m_ShaderStorageBufferUploadCI.pAllocator->AccessData(m_ShaderStorageBufferUpload->GetAllocation(), size, data);
			}

			inline const miru::base::BufferRef& GetCPUBuffer() const override { return m_ShaderStorageBufferUpload; };
			inline const miru::base::BufferRef& GetGPUBuffer() const override { return m_ShaderStorageBuffer; };
			inline const miru::base::BufferViewRef& GetCPUBufferView() const override { return m_ShaderStorageBufferViewUpload; };
			inline const miru::base::BufferViewRef& GetGPUBufferView() const override { return m_ShaderStorageBufferView; };

			inline size_t GetSize() { return sizeof(T); }
		};
	}
}