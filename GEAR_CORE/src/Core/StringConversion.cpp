#include "gear_core_common.h"
#include "StringConversion.h"
#include <cwctype>

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

std::string gear::core::toupper(const std::string& string)
{
	std::string str = string;
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) { return std::toupper(c); });
	return str;
}

std::wstring gear::core::towupper(const std::wstring& wstring)
{
	std::wstring wstr = wstring;
	std::transform(wstr.begin(), wstr.end(), wstr.begin(),
		[](wchar_t c) { return std::towupper(c); });
	return wstr;
}

std::string gear::core::tolower(const std::string& string)
{
	std::string str = string;
	std::transform(str.begin(), str.end(), str.begin(),
		[](unsigned char c) { return std::tolower(c); });
	return str;
}

std::wstring gear::core::towlower(const std::wstring& wstring)
{
	std::wstring wstr = wstring;
	std::transform(wstr.begin(), wstr.end(), wstr.begin(),
		[](wchar_t c) { return std::towlower(c); });
	return wstr;
}