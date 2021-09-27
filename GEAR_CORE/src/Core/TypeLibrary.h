#pragma once
#include "gear_core_common.h"
#include "UUID.h"

namespace gear 
{
namespace core 
{
	template<class T>
	class TypeLibrary
	{
	private:
		static std::map<UUID, Ref<T>> s_Entries;
	
	public:
		static void Insert(const UUID& uuid, const Ref<T>& component)
		{
			s_Entries[uuid] = component;
		}
		static void Insert(const UUID& uuid, Ref<T>&& component)
		{
			s_Entries[uuid] = std::move(component);
		}
		template<typename... Args>
		static void Emplace(Args&&... args)
		{
			s_Entries.emplace(std::forward<Args>(args)...);
		}
		static bool Has(const UUID& uuid)
		{
			auto it = s_Entries.find(uuid);
			if (it != s_Entries.end())
			{
				return true;
			}
			return false;
		}
		static Ref<T>& Get(const UUID& uuid)
		{
			return s_Entries[uuid];
		}
		static bool FindUUID(const Ref<T>& type, UUID& outUUID)
		{
			for (const auto& entry : s_Entries)
			{
				if (entry.second == type)
				{
					outUUID = entry.first;
					return true;
				}
			}
			return false;
		}
		static void Erase(const UUID& uuid)
		{
			auto it = s_Entries.find(uuid);
			if (it != s_Entries.end())
			{
				s_Entries.erase(it);
			}
		}
		static bool Empty()
		{
			return s_Entries.empty();
		}
		static size_t Size()
		{
			return s_Entries.size();
		}
		static std::map<UUID, Ref<T>>& GetMap()
		{
			return s_Entries;
		}
	};

	template<class T>
	std::map<UUID, Ref<T>> TypeLibrary<T>::s_Entries;

}
}
