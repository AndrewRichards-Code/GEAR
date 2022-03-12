#pragma once
#include "gear_core_common.h"

namespace gear 
{
namespace graphics
{
	class GEAR_API AllocatorManager
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
			Ref<miru::crossplatform::Context>			pContext;
			miru::crossplatform::Allocator::BlockSize	defaultBlockSize;
			bool										forceInitialisation = false;
		};

	private:
		static Ref<miru::crossplatform::Allocator> s_CPUAllocator;
		static Ref<miru::crossplatform::Allocator> s_GPUAllocator;
		static CreateInfo s_CI;
		static bool s_Initialised;

	public:
		static void Initialise(CreateInfo* pCreateInfo);
		static void Uninitialise();

		static Ref<miru::crossplatform::Allocator> GetAllocator(AllocatorType type);
		static Ref<miru::crossplatform::Allocator> GetCPUAllocator();
		static Ref<miru::crossplatform::Allocator> GetGPUAllocator();
		static void PrintMemoryBlockStatus();
		static const CreateInfo& GetCreateInfo() { return s_CI; }
	};
}
}