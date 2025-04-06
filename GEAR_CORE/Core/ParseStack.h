#pragma once
#include "gear_core_common.h"

namespace gear
{
	namespace core
	{
		template<typename _Ty>
		size_t GetDifferingIndex(const std::stack<_Ty>& a, const std::stack<_Ty>& b)
		{
			const size_t minCount = std::min(a.size(), b.size());
			size_t differingIndex = 0;
			for (size_t i = 0; i < minCount; i++)
			{
				differingIndex = i + 1;
				if (a._Get_container()[i] != b._Get_container()[i])
				{
					differingIndex = i;
					break;
				}
			}
			return differingIndex;
		}

		template<typename _Ty>
		void PopStackToDifferingIndex(std::stack<_Ty>& a, const size_t& differingIndex, std::function<void()> function)
		{
			while (a.size() != differingIndex)
			{
				a.pop();
				function();
			}
		}

		template<typename _Ty>
		void PopStack(std::stack<_Ty>& a, std::function<void()> function)
		{
			while (a.size())
			{
				a.pop();
				function();
			}
		}

		template<typename _Ty>
		void PushStackFromDifferingIndex(std::stack<_Ty>& writeStack, const std::stack<_Ty>& readStack, const size_t& differingIndex,
			std::function<void(const _Ty&)> function)
		{
			for (size_t i = differingIndex; i < readStack.size(); i++)
			{
				const _Ty& value = readStack._Get_container()[i];
				writeStack.push(value);
				function(value);
			}
		}

		template<typename _Ty>
		void ResolveStacks(std::stack<_Ty>& writeStack, const std::stack<_Ty>& readStack,
			std::function<void()> popFunction, std::function<void(const _Ty&)> pushFunction)
		{
			size_t differingIndex = core::GetDifferingIndex(writeStack, readStack);
			core::PopStackToDifferingIndex(writeStack, differingIndex, popFunction);
			core::PushStackFromDifferingIndex<std::string>(writeStack, readStack, differingIndex, pushFunction);
		}

		template<typename _Ty>
		void ClearStack(std::stack<_Ty>& writeStack, std::function<void()> popFunction)
		{
			PopStack(writeStack, popFunction);
		}
	}
}