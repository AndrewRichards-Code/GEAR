#include "gear_core_common.h"
#include "string_conversion.h"

std::string gear::core::to_string(const std::wstring& wstring)
{
	char* str = new char[wstring.size() + 1];
	wcstombs_s(nullptr, str, wstring.size() + 1, wstring.c_str(), wstring.size() + 1);
	std::string result(str);
	delete[] str;
	return result;
}

std::wstring gear::core::to_wstring(const std::string& string)
{
	wchar_t* wstr = new wchar_t[string.size() + 1];
	mbstowcs_s(nullptr, wstr, string.size() + 1, string.c_str(), string.size() + 1);
	std::wstring result(wstr);
	delete[] wstr;
	return result;
}
