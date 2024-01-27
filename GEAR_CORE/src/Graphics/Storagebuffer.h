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
		class GEAR_API Storagebuffer : public BaseStoragebuffer
		{
		public:
			struct CreateInfo
			{
				std::string debugName;
				void* device;
				void* data;
				uint32_t count;
			};

		public:
			std::vector<T> m_Data;
		
		private:
			miru::base::BufferRef m_StorageBuffer, m_StorageBufferUpload;
			miru::base::BufferViewRef m_StorageBufferView, m_StorageBufferViewUpload;

			CreateInfo m_CI;

		public:
			Storagebuffer(CreateInfo* pCreateInfo)
			{
				m_CI = *pCreateInfo;

				miru::base::Buffer::CreateInfo storageBufferCI, storageBufferUploadCI;
				miru::base::BufferView::CreateInfo storageBufferViewCI, storageBufferViewUploadCI;

				storageBufferUploadCI.debugName = "GEAR_CORE_StoragebufferUpload: " + m_CI.debugName;
				storageBufferUploadCI.device = m_CI.device;
				storageBufferUploadCI.usage = miru::base::Buffer::UsageBit::TRANSFER_SRC_BIT | miru::base::Buffer::UsageBit::TRANSFER_DST_BIT;
				storageBufferUploadCI.size = GetSize();
				storageBufferUploadCI.data = m_CI.data;
				storageBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
				m_StorageBufferUpload = miru::base::Buffer::Create(&storageBufferUploadCI);

				storageBufferCI.debugName = "GEAR_CORE_Storagebuffer: " + m_CI.debugName;
				storageBufferCI.device = m_CI.device;
				storageBufferCI.usage = miru::base::Buffer::UsageBit::TRANSFER_SRC_BIT | miru::base::Buffer::UsageBit::TRANSFER_DST_BIT | miru::base::Buffer::UsageBit::STORAGE_BIT;
				storageBufferCI.size = GetSize();
				storageBufferCI.data = nullptr;
				storageBufferCI.allocator = AllocatorManager::GetGPUAllocator();
				m_StorageBuffer = miru::base::Buffer::Create(&storageBufferCI);

				storageBufferViewUploadCI.debugName = "GEAR_CORE_StoragebufferViewUpload: " + m_CI.debugName;
				storageBufferViewUploadCI.device = m_CI.device;
				storageBufferViewUploadCI.type = miru::base::BufferView::Type::STORAGE;
				storageBufferViewUploadCI.buffer = m_StorageBufferUpload;
				storageBufferViewUploadCI.offset = 0;
				storageBufferViewUploadCI.size = GetSize();
				storageBufferViewUploadCI.stride = sizeof(T);
				m_StorageBufferViewUpload = miru::base::BufferView::Create(&storageBufferViewUploadCI);

				storageBufferViewCI.debugName = "GEAR_CORE_StoragebufferView: " + m_CI.debugName;
				storageBufferViewCI.device = m_CI.device;
				storageBufferViewCI.type = miru::base::BufferView::Type::STORAGE;
				storageBufferViewCI.buffer = m_StorageBuffer;
				storageBufferViewCI.offset = 0;
				storageBufferViewCI.size = GetSize();
				storageBufferViewCI.stride = sizeof(T);
				m_StorageBufferView = miru::base::BufferView::Create(&storageBufferViewCI);

				m_Data.resize(m_CI.count);
				memcpy(m_Data.data(), m_CI.data, GetSize());
			}
			~Storagebuffer() {}

			const CreateInfo& GetCreateInfo() { return m_CI; }

			void SubmitData() const
			{
				m_StorageBufferUpload->GetCreateInfo().allocator->SubmitData(m_StorageBufferUpload->GetAllocation(), 0, GetSize(), (void*)m_Data.data());
			}
			void AccessData() const
			{
				m_StorageBufferUpload->GetCreateInfo().allocator->AccessData(m_StorageBufferUpload->GetAllocation(), 0, GetSize(), (void*)m_Data.data());
			}

			inline T& operator[](size_t index) { return m_Data[index]; }
			inline const T& operator[](size_t index) const { return m_Data[index]; }

			inline const miru::base::BufferRef& GetCPUBuffer() const override { return m_StorageBufferUpload; };
			inline const miru::base::BufferRef& GetGPUBuffer() const override { return m_StorageBuffer; };
			inline const miru::base::BufferViewRef& GetCPUBufferView() const override { return m_StorageBufferViewUpload; };
			inline const miru::base::BufferViewRef& GetGPUBufferView() const override { return m_StorageBufferView; };

			inline size_t GetSize() const { return sizeof(T) * m_CI.count; }
		};
	}
}