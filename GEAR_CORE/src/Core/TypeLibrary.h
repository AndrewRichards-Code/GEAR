#pragma once
#include "gear_core_common.h"

namespace gear {
namespace core {

 template<class T>
class TypeLibrary
{
private:
    static std::map<std::string, gear::Ref<T>> s_Entries;

public:
    static void Insert(const std::string& name, const gear::Ref<T>& component)
    {
        s_Entries[name] =  component;
    }
    static void Insert(const std::string& name, gear::Ref<T>&& component)
    {
        s_Entries[name] = std::move(component);
    }
    template<typename... Args>
    static void Emplace(Args&&... args)
    {
        s_Entries.emplace(std::forward<Args>(args)...);
    }
    static bool Has(const std::string& name)
    {
        auto it = s_Entries.find(name);
        if (it != s_Entries.end())
        {
            return true;
        }
        return false;
    }
    static const gear::Ref<T>& CFind(const std::string& name)
    {
        return s_Entries[name];
    }
    static gear::Ref<T>& Find(const std::string& name)
    {
        return s_Entries[name];
    }
    static void Erase(const std::string& name)
    {
        auto it = s_Entries.find(name);
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
    static std::map<std::string, gear::Ref<T>>& GetMap()
    {
        return s_Entries;
    }
};

template<class T>
std::map<std::string, gear::Ref<T>> TypeLibrary<T>::s_Entries;

}
}
