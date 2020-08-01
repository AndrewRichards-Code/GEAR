#pragma once

#include "gear_core_common.h"

namespace gear {
namespace graphics{
class MemoryBlockManager
{
public:
	enum class MemoryBlockType : uint32_t
	{
		UNKNOWN,
		CPU,
		GPU
	};

	struct CreateInfo
	{
		miru::Ref<miru::crossplatform::Context>		pContext;
		miru::crossplatform::MemoryBlock::BlockSize defaultBlockSize;
	};

private:
	static CreateInfo s_CI;
	static std::map<miru::Ref<miru::crossplatform::MemoryBlock>, std::string> s_MB_DebugNames;
	static uint32_t s_MB_CPU_ID;
	static uint32_t s_MB_GPU_ID;
	static bool s_Initialised;


public:
	static void Initialise(CreateInfo* pCreateInfo);
	static miru::Ref<miru::crossplatform::MemoryBlock> GetMemoryBlock(MemoryBlockType type, miru::crossplatform::MemoryBlock::BlockSize overrideBlockSize = miru::crossplatform::MemoryBlock::BlockSize(0));
	static void PrintMemoryBlockStatus();

private:
	static miru::Ref<miru::crossplatform::MemoryBlock> AddMemoryBlock(MemoryBlockType type, miru::crossplatform::MemoryBlock::BlockSize overrideBlockSize = miru::crossplatform::MemoryBlock::BlockSize(0));
	static MemoryBlockType GetMemoryBlockType(miru::Ref<miru::crossplatform::MemoryBlock> memoryBlock);
};
}
}