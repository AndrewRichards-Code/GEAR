#include "gear_core_common.h"
#include "Core/FileDialog.h"
#include "ARC/src/StringConversion.h"

#include <shobjidl.h>

std::string gear::core::FileDialog_Open(const char* filterName, const char* filterSpec)
{
#if defined(_WIN32)
	std::string path = "";

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog* pFileOpen;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			std::wstring wFilterName = arc::ToWString(filterName);
			std::wstring wFilterSpec = arc::ToWString(filterSpec);
			COMDLG_FILTERSPEC filterSpecs = { wFilterName.c_str(), wFilterSpec.c_str() };

			if (SUCCEEDED(pFileOpen->SetFileTypes(1, &filterSpecs)))
			{
				if (SUCCEEDED(pFileOpen->Show(NULL)))
				{
					IShellItem* pShellItem;
					if (SUCCEEDED(pFileOpen->GetResult(&pShellItem)))
					{
						LPWSTR _path;
						pShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &_path);
						if (_path)
							path = arc::ToString(std::wstring(_path));

						CoTaskMemFree(_path);
						pShellItem->Release();
					}
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}

	return path;
#else
	return std::string();
#endif
}
std::string gear::core::FileDialog_Save(const char* filterName, const char* filterSpec)
{
#if defined(_WIN32)
	std::string path = "";

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog* pFileSave;
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL, IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

		if (SUCCEEDED(hr))
		{
			std::wstring wFilterName = arc::ToWString(filterName);
			std::wstring wFilterSpec = arc::ToWString(filterSpec);
			COMDLG_FILTERSPEC filterSpecs = { wFilterName.c_str(), wFilterSpec.c_str() };

			if (SUCCEEDED(pFileSave->SetFileTypes(1, &filterSpecs)))
			{
				if (SUCCEEDED(pFileSave->Show(NULL)))
				{
					IShellItem* pShellItem;
					if (SUCCEEDED(pFileSave->GetResult(&pShellItem)))
					{
						LPWSTR _path;
						pShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &_path);
						if (_path)
							path = arc::ToString(std::wstring(_path));

						CoTaskMemFree(_path);
						pShellItem->Release();
					}
				}
			}
			pFileSave->Release();
		}
		CoUninitialize();
	}

	return path;
#else
	return std::string();
#endif
}

std::string gear::core::FolderDialog_Browse()
{
#if defined(_WIN32)
	std::string path = "";

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileDialog* pFileOpen;
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			DWORD dwOptions;
			if (SUCCEEDED(pFileOpen->GetOptions(&dwOptions)))
			{
				pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
			}

			if (SUCCEEDED(pFileOpen->Show(NULL)))
			{
				IShellItem* pShellItem;
				if (SUCCEEDED(pFileOpen->GetResult(&pShellItem)))
				{
					LPWSTR _path;
					pShellItem->GetDisplayName(SIGDN_DESKTOPABSOLUTEPARSING, &_path);
					if (_path)
						path = arc::ToString(std::wstring(_path));

					CoTaskMemFree(_path);
					pShellItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}

	return path;

#else
	return std::string();
#endif
}
