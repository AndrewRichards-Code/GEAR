#pragma once
#include "gear_core_common.h"

namespace gear
{
namespace input
{
	class InputInterface 
	{
	public:
		enum class API : uint32_t
		{
			UNKNOWN,
			XINPUT
		};
		struct CreateInfo
		{
			API			inputAPI;
			uint32_t	controllerIndex; //From 0 to 3.
		};

	private:
		CreateInfo m_CI;
		static API s_API;
		
	public:
		InputInterface(CreateInfo* pCreateInfo);
		~InputInterface();

		const CreateInfo& GetCreateInfo() { return m_CI; }

		static inline const API& GetAPI() { return s_API; }

	};
}
}
