#pragma once

#include "gear_core_common.h"
#include "Graphics/MemoryBlockManager.h"

namespace gear {
namespace graphics {
template<typename T>
class Uniformbuffer : public T
	{
	public:
		struct CreateInfo
		{
			std::string debugName;
			void*		device;
			void*		data;
		};

	private:
		miru::Ref<miru::crossplatform::Buffer> m_UniformBuffer, m_UniformBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_UniformBufferCI, m_UniformBufferUploadCI;

		miru::Ref<miru::crossplatform::BufferView> m_UniformBufferView;
		miru::crossplatform::BufferView::CreateInfo m_UniformBufferViewCI;

		std::string m_DebugName_UBUpload;
		std::string m_DebugName_UB;
		std::string m_DebugName_UBV;

		CreateInfo m_CI;
		bool m_Upload = false;

	public:
		Uniformbuffer(CreateInfo* pCreateInfo)
		{
			m_CI = *pCreateInfo;

			m_DebugName_UBUpload = "GEAR_CORE_UniformBufferUpload: " + m_CI.debugName;
			m_UniformBufferUploadCI.debugName = m_DebugName_UBUpload.c_str();
			m_UniformBufferUploadCI.device = m_CI.device;
			m_UniformBufferUploadCI.usage = miru::crossplatform::Buffer::UsageBit::TRANSFER_SRC;
			m_UniformBufferUploadCI.size = GetSize();
			m_UniformBufferUploadCI.data = m_CI.data;
			m_UniformBufferUploadCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::CPU);
			m_UniformBufferUpload = miru::crossplatform::Buffer::Create(&m_UniformBufferUploadCI);

			m_DebugName_UB = "GEAR_CORE_UniformBuffer: " + m_CI.debugName;
			m_UniformBufferCI.debugName = m_DebugName_UB.c_str();
			m_UniformBufferCI.device = m_CI.device;
			m_UniformBufferCI.usage = miru::crossplatform::Buffer::UsageBit::TRANSFER_DST | miru::crossplatform::Buffer::UsageBit::UNIFORM;
			m_UniformBufferCI.size = GetSize();
			m_UniformBufferCI.data = nullptr;
			m_UniformBufferCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::GPU);
			m_UniformBuffer = miru::crossplatform::Buffer::Create(&m_UniformBufferCI);

			m_DebugName_UBV= "GEAR_CORE_UniformBufferViewUsage: " + m_CI.debugName;
			m_UniformBufferViewCI.debugName = m_DebugName_UBV.c_str();
			m_UniformBufferViewCI.device = m_CI.device;
			m_UniformBufferViewCI.type = miru::crossplatform::BufferView::Type::UNIFORM;
			m_UniformBufferViewCI.pBuffer = m_UniformBuffer;
			m_UniformBufferViewCI.offset = 0;
			m_UniformBufferViewCI.size = GetSize();
			m_UniformBufferViewCI.stride = 0;
			m_UniformBufferView = miru::crossplatform::BufferView::Create(&m_UniformBufferViewCI);
		}
		~Uniformbuffer() {}

		void SubmitData() const
		{
			m_UniformBufferUploadCI.pMemoryBlock->SubmitData(m_UniformBufferUpload->GetResource(), GetSize(), (void*)this);
		}
		void Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0, bool force = false)
		{
			if (!m_Upload || force)
			{
				cmdBuffer->CopyBuffer(cmdBufferIndex, m_UniformBufferUpload, m_UniformBuffer, { {0, 0, GetSize()} });
				m_Upload = true;
			}
		}

		inline const miru::Ref<miru::crossplatform::Buffer>& GetBuffer() const { return m_UniformBuffer; };
		inline const miru::Ref<miru::crossplatform::BufferView>& GetBufferView() const { return m_UniformBufferView; };

		inline constexpr size_t GetSize() const { return sizeof(T); }
	};
}
}