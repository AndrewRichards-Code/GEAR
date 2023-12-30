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
			miru::base::BufferViewRef m_ShaderStorageBufferView, m_ShaderStorageBufferViewUpload;

			CreateInfo m_CI;

		public:
			Storagebuffer(CreateInfo* pCreateInfo)
			{
				m_CI = *pCreateInfo;

				miru::base::Buffer::CreateInfo shaderStorageBufferCI, shaderStorageBufferUploadCI;
				miru::base::BufferView::CreateInfo shaderStorageBufferViewCI, shaderStorageBufferViewUploadCI;

				shaderStorageBufferUploadCI.debugName = "GEAR_CORE_ShaderStorageBufferUpload: " + m_CI.debugName;
				shaderStorageBufferUploadCI.device = m_CI.device;
				shaderStorageBufferUploadCI.usage = miru::base::Buffer::UsageBit::TRANSFER_SRC_BIT | miru::base::Buffer::UsageBit::TRANSFER_DST_BIT;
				shaderStorageBufferUploadCI.size = GetSize();
				shaderStorageBufferUploadCI.data = m_CI.data;
				shaderStorageBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
				m_ShaderStorageBufferUpload = miru::base::Buffer::Create(&shaderStorageBufferUploadCI);

				shaderStorageBufferCI.debugName = "GEAR_CORE_ShaderStorageBuffer: " + m_CI.debugName;
				shaderStorageBufferCI.device = m_CI.device;
				shaderStorageBufferCI.usage = miru::base::Buffer::UsageBit::TRANSFER_SRC_BIT | miru::base::Buffer::UsageBit::TRANSFER_DST_BIT | miru::base::Buffer::UsageBit::STORAGE_BIT;
				shaderStorageBufferCI.size = GetSize();
				shaderStorageBufferCI.data = nullptr;
				shaderStorageBufferCI.allocator = AllocatorManager::GetGPUAllocator();
				m_ShaderStorageBuffer = miru::base::Buffer::Create(&shaderStorageBufferCI);

				shaderStorageBufferViewUploadCI.debugName = "GEAR_CORE_ShaderStorageViewUpload: " + m_CI.debugName;
				shaderStorageBufferViewUploadCI.device = m_CI.device;
				shaderStorageBufferViewUploadCI.type = miru::base::BufferView::Type::UNIFORM;
				shaderStorageBufferViewUploadCI.buffer = m_ShaderStorageBufferUpload;
				shaderStorageBufferViewUploadCI.offset = 0;
				shaderStorageBufferViewUploadCI.size = GetSize();
				shaderStorageBufferViewUploadCI.stride = 0;
				m_ShaderStorageBufferViewUpload = miru::base::BufferView::Create(&shaderStorageBufferViewUploadCI);

				shaderStorageBufferViewCI.debugName = "GEAR_CORE_ShaderStorageView: " + m_CI.debugName;
				shaderStorageBufferViewCI.device = m_CI.device;
				shaderStorageBufferViewCI.type = miru::base::BufferView::Type::UNIFORM;
				shaderStorageBufferViewCI.buffer = m_ShaderStorageBuffer;
				shaderStorageBufferViewCI.offset = 0;
				shaderStorageBufferViewCI.size = GetSize();
				shaderStorageBufferViewCI.stride = 0;
				m_ShaderStorageBufferView = miru::base::BufferView::Create(&shaderStorageBufferViewCI);

				//Account of vptr of the base class.
				memcpy((void*)((uint8_t*)this + 0x8), m_CI.data, sizeof(T));
			}
			~Storagebuffer() {}

			const CreateInfo& GetCreateInfo() { return m_CI; }

			void SubmitData() const
			{
				m_ShaderStorageBufferUpload->GetCreateInfo().pAllocator->SubmitData(m_ShaderStorageBufferUpload->GetAllocation(), GetSize(), (void*)((uint8_t*)this + 0x8)); //Account of vptr of the base class.
			}
			void AccessData() const
			{
				m_ShaderStorageBufferUpload->GetCreateInfo().pAllocator->AccessData(m_ShaderStorageBufferUpload->GetAllocation(), GetSize(), (void*)((uint8_t*)this + 0x8)); //Account of vptr of the base class.
			}

			inline const miru::base::BufferRef& GetCPUBuffer() const override { return m_ShaderStorageBufferUpload; };
			inline const miru::base::BufferRef& GetGPUBuffer() const override { return m_ShaderStorageBuffer; };
			inline const miru::base::BufferViewRef& GetCPUBufferView() const override { return m_ShaderStorageBufferViewUpload; };
			inline const miru::base::BufferViewRef& GetGPUBufferView() const override { return m_ShaderStorageBufferView; };

			inline size_t GetSize() { return sizeof(T); }
		};
	}
}