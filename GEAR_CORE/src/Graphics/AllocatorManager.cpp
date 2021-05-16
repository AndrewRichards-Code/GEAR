#include "gear_core_common.h"
#include "AllocatorManager.h"


using namespace gear;
using namespace graphics;

using namespace miru;
using namespace miru::crossplatform;

Ref<miru::crossplatform::Allocator> AllocatorManager::s_CPUAllocator = nullptr;
Ref<miru::crossplatform::Allocator> AllocatorManager::s_GPUAllocator = nullptr;
AllocatorManager::CreateInfo AllocatorManager::s_CI;
bool AllocatorManager::s_Initialised = false;

void AllocatorManager::Initialise(CreateInfo* pCreateInfo)
{
	if (s_Initialised)
		return;

	s_CI = *pCreateInfo;

	Allocator::CreateInfo allocatorCI;
	allocatorCI.debugName = "GEAR_CORE_CPU_ALLOCATOR";
	allocatorCI.pContext = s_CI.pContext;
	allocatorCI.blockSize = s_CI.defaultBlockSize;
	allocatorCI.properties = Allocator::PropertiesBit::HOST_VISIBLE_BIT | Allocator::PropertiesBit::HOST_COHERENT_BIT;
	s_CPUAllocator = Allocator::Create(&allocatorCI);

	allocatorCI.debugName = "GEAR_CORE_GPU_ALLOCATOR";
	allocatorCI.pContext = s_CI.pContext;
	allocatorCI.blockSize = s_CI.defaultBlockSize;
	allocatorCI.properties = Allocator::PropertiesBit::DEVICE_LOCAL_BIT;
	s_GPUAllocator = Allocator::Create(&allocatorCI);

	s_Initialised = true;
}

Ref<Allocator> AllocatorManager::GetAllocator(AllocatorType type)
{
	if (!s_Initialised)
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INIT_FAILED, "AllocatorManager has not been initialised.");
	}
	
	switch (type)
	{
	case AllocatorManager::AllocatorType::CPU:
		return s_CPUAllocator;
	case AllocatorManager::AllocatorType::GPU:
		return s_GPUAllocator;
	case AllocatorManager::AllocatorType::UNKNOWN:
	default:
	{
		GEAR_ASSERT(ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "AllocatorManager: Unknown AllocatorType.");
		return nullptr;
	}
	}
}

void AllocatorManager::PrintMemoryBlockStatus()
{
	/*if (GraphicsAPI::IsD3D12())
	{
		D3D12MA::Stats cpuAllocatorStats;
		D3D12MA::Stats gpuAllocatorStats;
		s_CPUAllocator->GetD3D12MAAllocator()->CalculateStats(&cpuAllocatorStats);
		s_GPUAllocator->GetD3D12MAAllocator()->CalculateStats(&gpuAllocatorStats);
	}
	else if (GraphicsAPI::IsVulkan())
	{
		VmaStats cpuAllocatorStats;
		VmaStats gpuAllocatorStats;
		s_CPUAllocator->GetVmaAllocator()->CalculateStats(&cpuAllocatorStats);
		s_GPUAllocator->GetVmaAllocator()->CalculateStats(&gpuAllocatorStats);
	}
	else
	{
		GEAR_ASSERT(Level::FATAL, ErrorCode::GRAPHICS | ErrorCode::INVALID_VALUE, "AllocatorManager: Unknown GraphicsAPI.");
	}*/

	/*for(auto& memoryBlock : MemoryBlock::GetMemoryBlocks())
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

		GEAR_PRINT_MESSAGE(Level::INFO, ErrorCode::OK, "%-48s(0x%x): - Usage(Bytes): %zu - %.3f%%.", name.c_str(), ptrVal, currentAllocatedSize, loadPercent);
	}*/
}
