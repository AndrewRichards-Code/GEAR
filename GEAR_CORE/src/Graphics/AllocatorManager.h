#pragma once

#include "gear_core_common.h"

namespace gear 
{
namespace graphics
{
	class AllocatorManager
	{
	public:
		enum class AllocatorType : uint32_t
		{
			UNKNOWN,
			CPU,
			GPU
		};

		struct CreateInfo
		{
			miru::Ref<miru::crossplatform::Context>		pContext;
			miru::crossplatform::Allocator::BlockSize	defaultBlockSize;
		};

	private:
		static miru::Ref<miru::crossplatform::Allocator> s_CPUAllocator;
		static miru::Ref<miru::crossplatform::Allocator> s_GPUAllocator;
		static CreateInfo s_CI;
		static bool s_Initialised;

	public:
		static void Initialise(CreateInfo* pCreateInfo);
		static miru::Ref<miru::crossplatform::Allocator> GetAllocator(AllocatorType type);
		static void PrintMemoryBlockStatus();
		static const CreateInfo& GetCreateInfo() { return s_CI; }
	};
}
}