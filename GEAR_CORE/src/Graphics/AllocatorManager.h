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
				miru::base::ContextRef				pContext;
				miru::base::Allocator::BlockSize	defaultBlockSize;
				bool								forceInitialisation = false;
			};

		private:
			static miru::base::AllocatorRef s_CPUAllocator;
			static miru::base::AllocatorRef s_GPUAllocator;
			static CreateInfo s_CI;
			static bool s_Initialised;

		public:
			static void Initialise(CreateInfo* pCreateInfo);
			static void Uninitialise();

			static miru::base::AllocatorRef GetAllocator(AllocatorType type);
			static miru::base::AllocatorRef GetCPUAllocator();
			static miru::base::AllocatorRef GetGPUAllocator();
			static void PrintMemoryBlockStatus();
			static const CreateInfo& GetCreateInfo() { return s_CI; }
		};
	}
}