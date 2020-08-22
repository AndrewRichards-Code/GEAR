#include "gear_core_common.h"
#include "MemoryBlockManager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

MemoryBlockManager::CreateInfo MemoryBlockManager::s_CI;
uint32_t MemoryBlockManager::s_MB_CPU_ID = 0;
uint32_t MemoryBlockManager::s_MB_GPU_ID = 0;
bool MemoryBlockManager::s_Initialised = false;

void MemoryBlockManager::Initialise(CreateInfo* pCreateInfo)
{
	if (s_Initialised)
		return;

	s_CI = *pCreateInfo;

	s_Initialised = true;
}

miru::Ref<MemoryBlock> MemoryBlockManager::GetMemoryBlock(MemoryBlockType type, MemoryBlock::BlockSize overrideBlockSize)
{
	if (!s_Initialised)
	{
		GEAR_ASSERT(core::Log::Level::FATAL, core::Log::ErrorCode::GRAPHICS | core::Log::ErrorCode::INIT_FAILED, "MemoryBlockManager has not been initialised.");
	}

	std::vector<miru::Ref<MemoryBlock>>& mbs = MemoryBlock::GetMemoryBlocks();
	miru::Ref<MemoryBlock> memoryBlock;

	for (auto it = mbs.rbegin(); it != mbs.rend(); it++)
	{
		MemoryBlockType it_type = GetMemoryBlockType(*it);
		if (it_type == type)
		{
			if ((*it)->GetCreateInfo().debugName.find("SwapchainDepthImage", 0) != std::string::npos)
				continue;

			memoryBlock = *it;
			break;
		}
	}

	if (!memoryBlock)
	{
		memoryBlock = AddMemoryBlock(type, overrideBlockSize);
	}
	return memoryBlock;
}

void MemoryBlockManager::PrintMemoryBlockStatus()
{
	for(auto& memoryBlock : MemoryBlock::GetMemoryBlocks())
	{
		uint64_t ptrVal = (uint64_t)memoryBlock.get();
		const std::string& name = memoryBlock->GetCreateInfo().debugName;
		size_t currentAllocatedSize = 0;
		if (memoryBlock->GetAllocatedResources()[memoryBlock].size())
		{
			currentAllocatedSize = memoryBlock->GetAllocatedResources()[memoryBlock].rbegin()->second.offset +
				memoryBlock->GetAllocatedResources()[memoryBlock].rbegin()->second.size;
		}
		float loadPercent = 100.0f * (float)currentAllocatedSize / (float)(memoryBlock->GetCreateInfo().blockSize);

		GEAR_PRINT_MESSAGE(core::Log::Level::INFO, core::Log::ErrorCode::OK, "%-48s(0x%x): - Usage(Bytes): %zu - %.3f%%.", name.c_str(), ptrVal, currentAllocatedSize, loadPercent);
	};
}

miru::Ref<MemoryBlock> MemoryBlockManager::AddMemoryBlock(MemoryBlockType type, MemoryBlock::BlockSize overrideBlockSize)
{
	bool bOverrideBlockSize = overrideBlockSize != MemoryBlock::BlockSize(0);

	MemoryBlock::CreateInfo mbCI;

	if (type == MemoryBlockType::CPU)
	{
		mbCI.debugName = "GEAR_CORE_MB_" + std::to_string(s_MB_CPU_ID) + "_CPU";
		mbCI.pContext = s_CI.pContext;
		mbCI.blockSize = bOverrideBlockSize ? overrideBlockSize : s_CI.defaultBlockSize;
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_ID++;
	}
	else if (type == MemoryBlockType::GPU)
	{
		mbCI.debugName = "GEAR_CORE_MB_" + std::to_string(s_MB_GPU_ID) + "_GPU";
		mbCI.pContext = s_CI.pContext;
		mbCI.blockSize = bOverrideBlockSize ? overrideBlockSize : s_CI.defaultBlockSize;
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_ID++;
	}
	else
	{
		GEAR_ASSERT(core::Log::Level::FATAL, core::Log::ErrorCode::GRAPHICS | core::Log::ErrorCode::INVALID_VALUE, "MemoryBlockManager: Unknown MemoryBlockType.");
	}

	miru::Ref<MemoryBlock> memoryBlock = MemoryBlock::Create(&mbCI);
	return memoryBlock;
}

MemoryBlockManager::MemoryBlockType MemoryBlockManager::GetMemoryBlockType(miru::Ref<MemoryBlock> memoryBlock)
{
	const MemoryBlock::PropertiesBit& properties = memoryBlock->GetCreateInfo().properties;

	if (properties == (MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT))
	{
		return MemoryBlockType::CPU;
	}
	else if (properties == MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT)
	{
		return MemoryBlockType::GPU;
	}
	else
	{
		GEAR_ASSERT(core::Log::Level::FATAL, core::Log::ErrorCode::GRAPHICS | core::Log::ErrorCode::INVALID_VALUE, "MemoryBlockManager: Unknown MemoryBlockType.");
		return MemoryBlockType::UNKNOWN;
	}
}