#pragma once

#include "gear_core_common.h"
#include "Graphics/AllocatorManager.h"
#include "Graphics/UniformBufferStructures.h"

namespace gear 
{
	namespace graphics
	{
		class GEAR_API BaseUniformbuffer
		{
		public:
			virtual ~BaseUniformbuffer() = default;

			virtual const miru::base::BufferRef& GetCPUBuffer() const = 0;
			virtual const miru::base::BufferRef& GetGPUBuffer() const = 0;
			virtual const miru::base::BufferViewRef& GetCPUBufferView() const = 0;
			virtual const miru::base::BufferViewRef& GetGPUBufferView() const = 0;
		};

		template<typename T>
		class GEAR_API Uniformbuffer : public T, public BaseUniformbuffer
		{
		public:
			struct CreateInfo
			{
				std::string debugName;
				void* device;
				void* data;
			};

		private:
			miru::base::BufferRef m_UniformBuffer, m_UniformBufferUpload;
			miru::base::Buffer::CreateInfo m_UniformBufferCI, m_UniformBufferUploadCI;

			miru::base::BufferViewRef m_UniformBufferView, m_UniformBufferViewUpload;
			miru::base::BufferView::CreateInfo m_UniformBufferViewCI, m_UniformBufferViewUploadCI;

			CreateInfo m_CI;

		public:
			Uniformbuffer(CreateInfo* pCreateInfo)
			{
				m_CI = *pCreateInfo;

				m_UniformBufferUploadCI.debugName = "GEAR_CORE_UniformBufferUpload: " + m_CI.debugName;
				m_UniformBufferUploadCI.device = m_CI.device;
				m_UniformBufferUploadCI.usage = miru::base::Buffer::UsageBit::TRANSFER_SRC_BIT;
				m_UniformBufferUploadCI.size = GetSize();
				m_UniformBufferUploadCI.data = m_CI.data;
				m_UniformBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
				m_UniformBufferUpload = miru::base::Buffer::Create(&m_UniformBufferUploadCI);

				m_UniformBufferCI.debugName = "GEAR_CORE_UniformBuffer: " + m_CI.debugName;
				m_UniformBufferCI.device = m_CI.device;
				m_UniformBufferCI.usage = miru::base::Buffer::UsageBit::TRANSFER_DST_BIT | miru::base::Buffer::UsageBit::UNIFORM_BIT;
				m_UniformBufferCI.size = GetSize();
				m_UniformBufferCI.data = nullptr;
				m_UniformBufferCI.allocator = AllocatorManager::GetGPUAllocator();
				m_UniformBuffer = miru::base::Buffer::Create(&m_UniformBufferCI);

				m_UniformBufferViewUploadCI.debugName = "GEAR_CORE_UniformBufferViewUpload: " + m_CI.debugName;
				m_UniformBufferViewUploadCI.device = m_CI.device;
				m_UniformBufferViewUploadCI.type = miru::base::BufferView::Type::UNIFORM;
				m_UniformBufferViewUploadCI.buffer = m_UniformBufferUpload;
				m_UniformBufferViewUploadCI.offset = 0;
				m_UniformBufferViewUploadCI.size = GetSize();
				m_UniformBufferViewUploadCI.stride = 0;
				m_UniformBufferViewUpload = miru::base::BufferView::Create(&m_UniformBufferViewUploadCI);

				m_UniformBufferViewCI.debugName = "GEAR_CORE_UniformBufferView: " + m_CI.debugName;
				m_UniformBufferViewCI.device = m_CI.device;
				m_UniformBufferViewCI.type = miru::base::BufferView::Type::UNIFORM;
				m_UniformBufferViewCI.buffer = m_UniformBuffer;
				m_UniformBufferViewCI.offset = 0;
				m_UniformBufferViewCI.size = GetSize();
				m_UniformBufferViewCI.stride = 0;
				m_UniformBufferView = miru::base::BufferView::Create(&m_UniformBufferViewCI);
			}
			~Uniformbuffer() {}

			const CreateInfo& GetCreateInfo() { return m_CI; }

			void SubmitData() const
			{
				m_UniformBufferUploadCI.allocator->SubmitData(m_UniformBufferUpload->GetAllocation(), 0, GetSize(), (void*)((uint8_t*)this + 0x8)); //Account of vptr of the base class.
			}

			inline const miru::base::BufferRef& GetCPUBuffer() const override { return m_UniformBufferUpload; };
			inline const miru::base::BufferRef& GetGPUBuffer() const override { return m_UniformBuffer; };
			inline const miru::base::BufferViewRef& GetCPUBufferView() const override { return m_UniformBufferViewUpload; };
			inline const miru::base::BufferViewRef& GetGPUBufferView() const override { return m_UniformBufferView; };

			inline constexpr size_t GetSize() const { return sizeof(T); }
		};
	}
}