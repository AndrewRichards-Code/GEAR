#pragma once
#include "UI/Panels/BasePanel.h"

typedef void* ImTextureID;

namespace gear
{
	namespace graphics
	{
		class Texture;
	}
	namespace ui
	{
		namespace panels
		{
			class GEAR_UI_API ContentBrowserPanel final : public Panel
			{
				//enums/structs
			public:
				struct CreateInfo
				{
					std::filesystem::path AssetFolderPath;
				};

				enum class FileState : uint8_t
				{
					NORMAL = 0,
					CUT,
					COPY,
					RENAME
				};

				//Methods
			public:
				ContentBrowserPanel(CreateInfo* pCreateInfo);
				~ContentBrowserPanel();

				void Draw() override;

				bool DrawTopBar();
				void DrawIcons();
				void DrawIcon(const std::filesystem::directory_entry& directory);
				void DrawIconContextMenu(const std::filesystem::directory_entry& directory, bool hovered);
				void DrawContextMenu();

				void CreateFileDialog();
				void RenameFileDialog();
				void ImportAssetDialog();

				inline CreateInfo& GetCreateInfo() { return m_CI; }

				inline void SetCurrentPath(const std::filesystem::path& folderpath) { m_CurrentPath = folderpath; }

				//Members
			private:
				CreateInfo m_CI;
				std::filesystem::path m_CurrentPath;

				bool m_UploadFolderAndFileTextures = true;
				Ref<graphics::Texture> m_FolderTexture;
				Ref<graphics::Texture> m_FileTexture;
				const std::filesystem::path m_FolderTextureFilepath = "icons8-folder-512.png";
				const std::filesystem::path m_FileTextureFilepath = "icons8-file-512.png";

				std::function<void(const std::filesystem::path&)> m_PopupWindowCloseCallback = nullptr;
				std::function<void()> m_PopupWindowFunction = nullptr;
				std::filesystem::path m_PopupWindowFilepath = "";

				std::unordered_map<std::filesystem::path, FileState> m_ModifyingFiles;

				struct FileExtTexture
				{
					std::filesystem::path	filename;
					std::filesystem::path	extension;
					Ref<graphics::Texture>	texture = nullptr;
				};
				std::vector<FileExtTexture> m_FileExtTextureFilepath =
				{
					{"icons8-file-512-a.png",		".a"},
					{"icons8-file-512-aif.png",		".aif"},
					{"icons8-file-512-avi.png",		".avi"},
					{"icons8-file-512-bak.png",		".bak"},
					{"icons8-file-512-bat.png",		".bat"},
					{"icons8-file-512-bin.png",		".bin"},
					{"icons8-file-512-c.png",		".c"},
					{"icons8-file-512-cmake.png",	".cmake"},
					{"icons8-file-512-cpp.png",		".cpp"},
					{"icons8-file-512-cs.png",		".cs"},
					{"icons8-file-512-cso.png",		".cso"},
					{"icons8-file-512-cxx.png",		".cxx"},
					{"icons8-file-512-dll.png",		".dll"},
					{"icons8-file-512-dmp.png",		".dmp"},
					{"icons8-file-512-exe.png",		".exe"},
					{"icons8-file-512-exr.png",		".exr"},
					{"icons8-file-512-exp.png",		".exp"},
					{"icons8-file-512-fbx.png",		".fbx"},
					{"icons8-file-512-filters.png",	".filters"},
					{"icons8-file-512-h.png",		".h"},
					{"icons8-file-512-hdr.png",		".hdr"},
					{"icons8-file-512-hlsl.png",	".hlsl"},
					{"icons8-file-512-hpp.png",		".hpp"},
					{"icons8-file-512-html.png",	".html"},
					{"icons8-file-512-icf.png",		".icf"},
					{"icons8-file-512-ico.png",		".ico"},
					{"icons8-file-512-idb.png",		".idb"},
					{"icons8-file-512-ilk.png",		".ilk"},
					{"icons8-file-512-ini.png",		".ini"},
					{"icons8-file-512-inl.png",		".inl"},
					{"icons8-file-512-iobj.png",	".iobj"},
					{"icons8-file-512-ipdb.png",	".ipdb"},
					{"icons8-file-512-iso.png",		".iso"},
					{"icons8-file-512-ixx.png",		".ixx"},
					{"icons8-file-512-java.png",	".java"},
					{"icons8-file-512-jpg.png",		".jpg"},
					{"icons8-file-512-json.png",	".json"},
					{"icons8-file-512-lib.png",		".lib"},
					{"icons8-file-512-lnk.png",		".lnk"},
					{"icons8-file-512-log.png",		".log"},
					{"icons8-file-512-md.png",		".md"},
					{"icons8-file-512-midi.png",	".midi"},
					{"icons8-file-512-mp3.png",		".mp3"},
					{"icons8-file-512-mp4.png",		".mp4"},
					{"icons8-file-512-o.png",		".o"},
					{"icons8-file-512-obj.png",		".obj"},
					{"icons8-file-512-ogg.png",		".ogg"},
					{"icons8-file-512-pch.png",		".pch"},
					{"icons8-file-512-pdb.png",		".pdb"},
					{"icons8-file-512-pdf.png",		".pdf"},
					{"icons8-file-512-png.png",		".png"},
					{"icons8-file-512-psd.png",		".psd"},
					{"icons8-file-512-py.png",		".py"},
					{"icons8-file-512-rar.png",		".rar"},
					{"icons8-file-512-sh.png",		".sh"},
					{"icons8-file-512-sln.png",		".sln"},
					{"icons8-file-512-spv.png",		".spv"},
					{"icons8-file-512-svg.png",		".svg"},
					{"icons8-file-512-tga.png",		".tga"},
					{"icons8-file-512-ttf.png",		".ttf"},
					{"icons8-file-512-txt.png",		".txt"},
					{"icons8-file-512-user.png",	".user"},
					{"icons8-file-512-vcxproj.png",	".vcxproj"},
					{"icons8-file-512-wav.png",		".wav"},
					{"icons8-file-512-wma.png",		".wma"},
					{"icons8-file-512-wmv.png",		".wmv"},
					{"icons8-file-512-xml.png",		".xml"},
					{"icons8-file-512-yml.png",		".yml"},
					{"icons8-file-512-zip.png",		".zip"}
				};

			};
		}
	}
}