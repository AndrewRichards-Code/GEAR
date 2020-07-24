#include "gear_core_common.h"
#include "memoryblockmanager.h"

using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

MemoryBlockManager::CreateInfo MemoryBlockManager::s_CI;
std::map<miru::Ref<miru::crossplatform::MemoryBlock>, std::string> MemoryBlockManager::s_MB_DebugNames;
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
		GEAR_ASSERT(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_INIT_FAILED, "ERROR: gear::graphics::MemoryBlockManager: MemoryBlockManager has not been initialised.");
	}

	std::vector<miru::Ref<MemoryBlock>>& mbs = MemoryBlock::GetMemoryBlocks();
	miru::Ref<MemoryBlock> memoryBlock;

	for (auto it = mbs.rbegin(); it != mbs.rend(); it++)
	{
		MemoryBlockType it_type = GetMemoryBlockType(*it);
		if (it_type == type)
		{
			if (std::string((*it)->GetCreateInfo().debugName).find("SwapchainDepthImage", 0) != std::string::npos)
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
		const char* name = memoryBlock->GetCreateInfo().debugName;
		size_t currentAllocatedSize = 0;
		if (memoryBlock->GetAllocatedResources()[memoryBlock].size())
		{
			currentAllocatedSize = memoryBlock->GetAllocatedResources()[memoryBlock].rbegin()->second.offset +
				memoryBlock->GetAllocatedResources()[memoryBlock].rbegin()->second.size;
		}
		float loadPercent = 100.0f * (float)currentAllocatedSize / (float)(memoryBlock->GetCreateInfo().blockSize);

		std::cout << name << "(" << ptrVal << "): " << " - Usage(Bytes): " << currentAllocatedSize << " - " << loadPercent << "%.\n";
	};
}

miru::Ref<MemoryBlock> MemoryBlockManager::AddMemoryBlock(MemoryBlockType type, miru::crossplatform::MemoryBlock::BlockSize overrideBlockSize)
{
	bool bOverrideBlockSize = overrideBlockSize != miru::crossplatform::MemoryBlock::BlockSize(0);

	MemoryBlock::CreateInfo mbCI;
	std::string debugName;

	if (type == MemoryBlockType::CPU)
	{
		debugName = "GEAR_CORE_MB_" + std::to_string(s_MB_CPU_ID) + "_CPU";
		mbCI.debugName = debugName.c_str();
		mbCI.pContext = s_CI.pContext;
		mbCI.blockSize = bOverrideBlockSize ? overrideBlockSize : s_CI.defaultBlockSize;
		mbCI.properties = MemoryBlock::PropertiesBit::HOST_VISIBLE_BIT | MemoryBlock::PropertiesBit::HOST_COHERENT_BIT;
		s_MB_CPU_ID++;
	}
	else if (type == MemoryBlockType::GPU)
	{
		debugName = "GEAR_CORE_MB_" + std::to_string(s_MB_GPU_ID) + "_GPU";
		mbCI.debugName = debugName.c_str();
		mbCI.pContext = s_CI.pContext;
		mbCI.blockSize = bOverrideBlockSize ? overrideBlockSize : s_CI.defaultBlockSize;
		mbCI.properties = MemoryBlock::PropertiesBit::DEVICE_LOCAL_BIT;
		s_MB_GPU_ID++;
	}
	else
	{
		GEAR_ASSERT(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_INVALID_VALUE, "ERROR: gear::graphics::MemoryBlockManager: Unknown MemoryBlockType.");
	}

	miru::Ref<MemoryBlock> memoryBlock = MemoryBlock::Create(&mbCI);
	s_MB_DebugNames[memoryBlock] = std::move(debugName);
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
		GEAR_ASSERT(GEAR_ERROR_CODE::GEAR_GRAPHICS | GEAR_ERROR_CODE::GEAR_INVALID_VALUE, "ERROR: gear::graphics::MemoryBlockManager: Unknown MemoryBlockType.");
		return MemoryBlockType::UNKNOWN;
	}
}