#pragma once

#include "gear_core_common.h"
#include "Graphics/AllocatorManager.h"
#include "Graphics/UniformBufferStructures.h"

namespace gear 
{
namespace graphics
{
	template<typename T>
	class GEAR_API Uniformbuffer : public T
	{
	public:
		struct CreateInfo
		{
			std::string debugName;
			void*		device;
			void*		data;
		};

	private:
		Ref<miru::crossplatform::Buffer> m_UniformBuffer, m_UniformBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_UniformBufferCI, m_UniformBufferUploadCI;

		Ref<miru::crossplatform::BufferView> m_UniformBufferView;
		miru::crossplatform::BufferView::CreateInfo m_UniformBufferViewCI;

		CreateInfo m_CI;

		bool m_Upload = false;

	public:
		Uniformbuffer(CreateInfo* pCreateInfo)
		{
			m_CI = *pCreateInfo;

			m_UniformBufferUploadCI.debugName = "GEAR_CORE_UniformBufferUpload: " + m_CI.debugName;
			m_UniformBufferUploadCI.device = m_CI.device;
			m_UniformBufferUploadCI.usage = miru::crossplatform::Buffer::UsageBit::TRANSFER_SRC_BIT;
			m_UniformBufferUploadCI.size = GetSize();
			m_UniformBufferUploadCI.data = m_CI.data;
			m_UniformBufferUploadCI.pAllocator = AllocatorManager::GetCPUAllocator();
			m_UniformBufferUpload = miru::crossplatform::Buffer::Create(&m_UniformBufferUploadCI);

			m_UniformBufferCI.debugName = "GEAR_CORE_UniformBuffer: " + m_CI.debugName;
			m_UniformBufferCI.device = m_CI.device;
			m_UniformBufferCI.usage = miru::crossplatform::Buffer::UsageBit::TRANSFER_DST_BIT | miru::crossplatform::Buffer::UsageBit::UNIFORM_BIT;
			m_UniformBufferCI.size = GetSize();
			m_UniformBufferCI.data = nullptr;
			m_UniformBufferCI.pAllocator = AllocatorManager::GetGPUAllocator();
			m_UniformBuffer = miru::crossplatform::Buffer::Create(&m_UniformBufferCI);

			m_UniformBufferViewCI.debugName = "GEAR_CORE_UniformBufferViewUsage: " + m_CI.debugName;
			m_UniformBufferViewCI.device = m_CI.device;
			m_UniformBufferViewCI.type = miru::crossplatform::BufferView::Type::UNIFORM;
			m_UniformBufferViewCI.pBuffer = m_UniformBuffer;
			m_UniformBufferViewCI.offset = 0;
			m_UniformBufferViewCI.size = GetSize();
			m_UniformBufferViewCI.stride = 0;
			m_UniformBufferView = miru::crossplatform::BufferView::Create(&m_UniformBufferViewCI);
		}
		~Uniformbuffer() {}

		const CreateInfo& GetCreateInfo() { return m_CI; }

		void SubmitData() const
		{
			m_UniformBufferUploadCI.pAllocator->SubmitData(m_UniformBufferUpload->GetAllocation(), GetSize(), (void*)this);
		}
		void Upload(const Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false)
		{
			if (!m_Upload || force)
			{
				cmdBuffer->CopyBuffer(cmdBufferIndex, m_UniformBufferUpload, m_UniformBuffer, { {0, 0, GetSize()} });
				m_Upload = true;
			}
		}

		inline const Ref<miru::crossplatform::Buffer>& GetBuffer() const { return m_UniformBuffer; };
		inline const Ref<miru::crossplatform::BufferView>& GetBufferView() const { return m_UniformBufferView; };

		inline constexpr size_t GetSize() const { return sizeof(T); }
	};
}
}