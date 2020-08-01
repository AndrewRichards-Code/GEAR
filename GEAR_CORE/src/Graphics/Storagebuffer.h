#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics {
template<typename T>
class Storagebuffer : public T
	{
	public:
		struct CreateInfo
		{
			const char* debugName;
			void*		device;
			void*		data;
		};

	private:
		miru::Ref<miru::crossplatform::Buffer> m_ShaderStorageBuffer, m_ShaderStorageBufferUpload;
		miru::crossplatform::Buffer::CreateInfo m_ShaderStorageBufferCI, m_ShaderStorageBufferUploadCI;

		miru::Ref<miru::crossplatform::BufferView> m_ShaderStorageBufferView;
		miru::crossplatform::BufferView::CreateInfo m_ShaderStorageBufferViewCI;

		std::string m_DebugName_SSBUpload;
		std::string m_DebugName_SSB;
		std::string m_DebugName_SSBV;

		CreateInfo m_CI;

	public:
		Storagebuffer(CreateInfo* pCreateInfo) 
		{
			m_CI = *pCreateInfo;

			m_DebugName_SSBUpload = std::string("GEAR_CORE_ShaderStorageBufferUpload: ") + m_CI.debugName;
			m_ShaderStorageBufferUploadCI.debugName = m_DebugName_SSBUpload.c_str();
			m_ShaderStorageBufferUploadCI.device = m_CI.device;
			m_ShaderStorageBufferUploadCI.usage = miru::crossplatform::Buffer::UsageBit::TRANSFER_SRC | miru::crossplatform::Buffer::UsageBit::TRANSFER_DST;
			m_ShaderStorageBufferUploadCI.size = GetSize();
			m_ShaderStorageBufferUploadCI.data = m_CI.data;
			m_ShaderStorageBufferUploadCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::CPU);
			m_ShaderStorageBufferUpload = miru::crossplatform::Buffer::Create(&m_ShaderStorageBufferUploadCI);

			m_DebugName_SSB = std::string("GEAR_CORE_ShaderStorageBuffer: ") + m_CI.debugName;
			m_ShaderStorageBufferCI.debugName = m_DebugName_SSB.c_str();
			m_ShaderStorageBufferCI.device = m_CI.device;
			m_ShaderStorageBufferCI.usage = miru::crossplatform::Buffer::UsageBit::TRANSFER_SRC | miru::crossplatform::Buffer::UsageBit::TRANSFER_DST | miru::crossplatform::Buffer::UsageBit::STORAGE;
			m_ShaderStorageBufferCI.size = GetSize();
			m_ShaderStorageBufferCI.data = nullptr;
			m_ShaderStorageBufferCI.pMemoryBlock = MemoryBlockManager::GetMemoryBlock(MemoryBlockManager::MemoryBlockType::GPU);
			m_ShaderStorageBuffer = miru::crossplatform::Buffer::Create(&m_ShaderStorageBufferCI);

			m_DebugName_SSBV = std::string("GEAR_CORE_ShaderStorageViewUsage: ") + m_CI.debugName;
			m_ShaderStorageBufferViewCI.debugName = m_DebugName_SSBV.c_str();
			m_ShaderStorageBufferViewCI.device = m_CI.device;
			m_ShaderStorageBufferViewCI.type = miru::crossplatform::BufferView::Type::UNIFORM;
			m_ShaderStorageBufferViewCI.pBuffer = m_ShaderStorageBuffer;
			m_ShaderStorageBufferViewCI.offset = 0;
			m_ShaderStorageBufferViewCI.size = GetSize();
			m_ShaderStorageBufferViewCI.stride = 0;
			m_ShaderStorageBufferView = miru::crossplatform::BufferView::Create(&m_ShaderStorageBufferViewCI);
		}
		~Storagebuffer() {}

		void SubmitData(const void* data, size_t  size) const
		{
			m_ShaderStorageBufferUploadCI.pMemoryBlock->SubmitData(m_ShaderStorageBufferUpload->GetResource(), (size_t)size, (void*)data);
		}
		void AccessData(void* data, size_t size) const 
		{
			m_ShaderStorageBufferUploadCI.pMemoryBlock->AccessData(m_ShaderStorageBufferUpload->GetResource(), size, data);
		}
		void Upload(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0)
		{
			cmdBuffer->CopyBuffer(cmdBufferIndex, m_ShaderStorageBufferUpload, m_ShaderStorageBuffer, { {0, 0, GetSize()} });
		}
		void Download(const miru::Ref<miru::crossplatform::CommandBuffer>& cmdBuffer, uint32_t cmdBufferIndex = 0)
		{
			cmdBuffer->CopyBuffer(cmdBufferIndex, m_ShaderStorageBuffer, m_ShaderStorageBufferUpload, { {0, 0, GetSize()} });
		}

		inline const miru::Ref<miru::crossplatform::Buffer>& GetBuffer() const { return m_ShaderStorageBuffer; };
		inline const miru::Ref<miru::crossplatform::BufferView>& GetBufferView() const { return m_ShaderStorageBufferView; };

		inline size_t GetSize() { return sizeof(T); }
	};
}
}