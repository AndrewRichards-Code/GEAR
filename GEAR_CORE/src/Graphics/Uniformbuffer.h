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
			miru::base::BufferViewRef m_UniformBufferView, m_UniformBufferViewUpload;

			CreateInfo m_CI;

		public:
			Uniformbuffer(CreateInfo* pCreateInfo)
			{
				m_CI = *pCreateInfo;

				miru::base::Buffer::CreateInfo uniformBufferCI, uniformBufferUploadCI;
				miru::base::BufferView::CreateInfo uniformBufferViewCI, uniformBufferViewUploadCI;

				uniformBufferUploadCI.debugName = "GEAR_CORE_UniformBufferUpload: " + m_CI.debugName;
				uniformBufferUploadCI.device = m_CI.device;
				uniformBufferUploadCI.usage = miru::base::Buffer::UsageBit::TRANSFER_SRC_BIT;
				uniformBufferUploadCI.size = GetSize();
				uniformBufferUploadCI.data = m_CI.data;
				uniformBufferUploadCI.allocator = AllocatorManager::GetCPUAllocator();
				m_UniformBufferUpload = miru::base::Buffer::Create(&uniformBufferUploadCI);

				uniformBufferCI.debugName = "GEAR_CORE_UniformBuffer: " + m_CI.debugName;
				uniformBufferCI.device = m_CI.device;
				uniformBufferCI.usage = miru::base::Buffer::UsageBit::TRANSFER_DST_BIT | miru::base::Buffer::UsageBit::UNIFORM_BIT;
				uniformBufferCI.size = GetSize();
				uniformBufferCI.data = nullptr;
				uniformBufferCI.allocator = AllocatorManager::GetGPUAllocator();
				m_UniformBuffer = miru::base::Buffer::Create(&uniformBufferCI);

				uniformBufferViewUploadCI.debugName = "GEAR_CORE_UniformBufferViewUpload: " + m_CI.debugName;
				uniformBufferViewUploadCI.device = m_CI.device;
				uniformBufferViewUploadCI.type = miru::base::BufferView::Type::UNIFORM;
				uniformBufferViewUploadCI.buffer = m_UniformBufferUpload;
				uniformBufferViewUploadCI.offset = 0;
				uniformBufferViewUploadCI.size = GetSize();
				uniformBufferViewUploadCI.stride = 0;
				m_UniformBufferViewUpload = miru::base::BufferView::Create(&uniformBufferViewUploadCI);

				uniformBufferViewCI.debugName = "GEAR_CORE_UniformBufferView: " + m_CI.debugName;
				uniformBufferViewCI.device = m_CI.device;
				uniformBufferViewCI.type = miru::base::BufferView::Type::UNIFORM;
				uniformBufferViewCI.buffer = m_UniformBuffer;
				uniformBufferViewCI.offset = 0;
				uniformBufferViewCI.size = GetSize();
				uniformBufferViewCI.stride = 0;
				m_UniformBufferView = miru::base::BufferView::Create(&uniformBufferViewCI);
				
				//Account of vptr of the base class.
				memcpy((void*)((uint8_t*)this + 0x8), m_CI.data, sizeof(T));
			}
			~Uniformbuffer() {}

			const CreateInfo& GetCreateInfo() { return m_CI; }

			void SubmitData() const
			{
				m_UniformBufferUpload->GetCreateInfo().allocator->SubmitData(m_UniformBufferUpload->GetAllocation(), 0, GetSize(), (void*)((uint8_t*)this + 0x8)); //Account of vptr of the base class.
			}

			inline const miru::base::BufferRef& GetCPUBuffer() const override { return m_UniformBufferUpload; };
			inline const miru::base::BufferRef& GetGPUBuffer() const override { return m_UniformBuffer; };
			inline const miru::base::BufferViewRef& GetCPUBufferView() const override { return m_UniformBufferViewUpload; };
			inline const miru::base::BufferViewRef& GetGPUBufferView() const override { return m_UniformBufferView; };

			inline constexpr size_t GetSize() const { return arc::Align<size_t>(sizeof(T), 256); } // Alignment requirements for D3D12 and Vulkan.
		};
	}
}