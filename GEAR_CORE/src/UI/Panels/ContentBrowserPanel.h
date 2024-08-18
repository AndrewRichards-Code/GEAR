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
			class GEAR_API ContentBrowserPanel final : public Panel
			{
				//enums/structs
			public:
				struct CreateInfo
				{
					std::filesystem::path AssetFolderPath;
				};

				//Methods
			public:
				ContentBrowserPanel(CreateInfo* pCreateInfo);
				~ContentBrowserPanel();

				void Draw() override;

				bool DrawTopBar();
				void DrawIcons();
				void DrawIcon(const std::filesystem::directory_entry& directory);
				void DrawIconContextMenu(const std::filesystem::directory_entry& directory);
				void DrawContextMenu();

				std::filesystem::path CreateFileDialog(const std::filesystem::path& filename, const std::filesystem::path& extension);
				void ImportAssetDialog(const std::filesystem::path& impotFilepath);

				inline CreateInfo& GetCreateInfo() { return m_CI; }

				inline void SetCurrentPath(const std::filesystem::path& folderpath) { m_CurrentPath = folderpath; }

				//Members
			private:
				CreateInfo m_CI;
				std::filesystem::path m_CurrentPath;

				bool m_UploadFolderAndFileTextures = true;
				Ref<graphics::Texture> m_FolderTexture;
				Ref<graphics::Texture> m_FileTexture;
				const std::filesystem::path m_FolderTextureFilepath = "GEARBOX/res/icons/icons8-folder-512.png";
				const std::filesystem::path m_FileTextureFilepath = "GEARBOX/res/icons/icons8-file-512.png";

				typedef void(ContentBrowserPanel::* PFN_PopupWindowFunction)(const std::filesystem::path&);
				PFN_PopupWindowFunction m_PopupWindowFunction = nullptr;
				std::filesystem::path m_PopupWindowFilepath = "";


				struct FileExtTexture
				{
					std::filesystem::path	filename;
					std::filesystem::path	extension;
					Ref<graphics::Texture>	texture = nullptr;
				};
				std::vector<FileExtTexture> m_FileExtTextureFilepath =
				{
					{"GEARBOX/res/icons/icons8-file-512-a.png",			".a"},
					{"GEARBOX/res/icons/icons8-file-512-aif.png",		".aif"},
					{"GEARBOX/res/icons/icons8-file-512-avi.png",		".avi"},
					{"GEARBOX/res/icons/icons8-file-512-bak.png",		".bak"},
					{"GEARBOX/res/icons/icons8-file-512-bat.png",		".bat"},
					{"GEARBOX/res/icons/icons8-file-512-bin.png",		".bin"},
					{"GEARBOX/res/icons/icons8-file-512-c.png",			".c"},
					{"GEARBOX/res/icons/icons8-file-512-cmake.png",		".cmake"},
					{"GEARBOX/res/icons/icons8-file-512-cpp.png",		".cpp"},
					{"GEARBOX/res/icons/icons8-file-512-cs.png",		".cs"},
					{"GEARBOX/res/icons/icons8-file-512-cso.png",		".cso"},
					{"GEARBOX/res/icons/icons8-file-512-cxx.png",		".cxx"},
					{"GEARBOX/res/icons/icons8-file-512-dll.png",		".dll"},
					{"GEARBOX/res/icons/icons8-file-512-dmp.png",		".dmp"},
					{"GEARBOX/res/icons/icons8-file-512-exe.png",		".exe"},
					{"GEARBOX/res/icons/icons8-file-512-exr.png",		".exr"},
					{"GEARBOX/res/icons/icons8-file-512-exp.png",		".exp"},
					{"GEARBOX/res/icons/icons8-file-512-fbx.png",		".fbx"},
					{"GEARBOX/res/icons/icons8-file-512-filters.png",	".filters"},
					{"GEARBOX/res/icons/icons8-file-512-h.png",			".h"},
					{"GEARBOX/res/icons/icons8-file-512-hdr.png",		".hdr"},
					{"GEARBOX/res/icons/icons8-file-512-hlsl.png",		".hlsl"},
					{"GEARBOX/res/icons/icons8-file-512-hpp.png",		".hpp"},
					{"GEARBOX/res/icons/icons8-file-512-html.png",		".html"},
					{"GEARBOX/res/icons/icons8-file-512-icf.png",		".icf"},
					{"GEARBOX/res/icons/icons8-file-512-ico.png",		".ico"},
					{"GEARBOX/res/icons/icons8-file-512-idb.png",		".idb"},
					{"GEARBOX/res/icons/icons8-file-512-ilk.png",		".ilk"},
					{"GEARBOX/res/icons/icons8-file-512-ini.png",		".ini"},
					{"GEARBOX/res/icons/icons8-file-512-inl.png",		".inl"},
					{"GEARBOX/res/icons/icons8-file-512-iobj.png",		".iobj"},
					{"GEARBOX/res/icons/icons8-file-512-ipdb.png",		".ipdb"},
					{"GEARBOX/res/icons/icons8-file-512-iso.png",		".iso"},
					{"GEARBOX/res/icons/icons8-file-512-ixx.png",		".ixx"},
					{"GEARBOX/res/icons/icons8-file-512-java.png",		".java"},
					{"GEARBOX/res/icons/icons8-file-512-jpg.png",		".jpg"},
					{"GEARBOX/res/icons/icons8-file-512-json.png",		".json"},
					{"GEARBOX/res/icons/icons8-file-512-lib.png",		".lib"},
					{"GEARBOX/res/icons/icons8-file-512-lnk.png",		".lnk"},
					{"GEARBOX/res/icons/icons8-file-512-log.png",		".log"},
					{"GEARBOX/res/icons/icons8-file-512-md.png",		".md"},
					{"GEARBOX/res/icons/icons8-file-512-midi.png",		".midi"},
					{"GEARBOX/res/icons/icons8-file-512-mp3.png",		".mp3"},
					{"GEARBOX/res/icons/icons8-file-512-mp4.png",		".mp4"},
					{"GEARBOX/res/icons/icons8-file-512-o.png",			".o"},
					{"GEARBOX/res/icons/icons8-file-512-obj.png",		".obj"},
					{"GEARBOX/res/icons/icons8-file-512-ogg.png",		".ogg"},
					{"GEARBOX/res/icons/icons8-file-512-pch.png",		".pch"},
					{"GEARBOX/res/icons/icons8-file-512-pdb.png",		".pdb"},
					{"GEARBOX/res/icons/icons8-file-512-pdf.png",		".pdf"},
					{"GEARBOX/res/icons/icons8-file-512-png.png",		".png"},
					{"GEARBOX/res/icons/icons8-file-512-psd.png",		".psd"},
					{"GEARBOX/res/icons/icons8-file-512-py.png",		".py"},
					{"GEARBOX/res/icons/icons8-file-512-rar.png",		".rar"},
					{"GEARBOX/res/icons/icons8-file-512-sh.png",		".sh"},
					{"GEARBOX/res/icons/icons8-file-512-sln.png",		".sln"},
					{"GEARBOX/res/icons/icons8-file-512-spv.png",		".spv"},
					{"GEARBOX/res/icons/icons8-file-512-svg.png",		".svg"},
					{"GEARBOX/res/icons/icons8-file-512-tga.png",		".tga"},
					{"GEARBOX/res/icons/icons8-file-512-ttf.png",		".ttf"},
					{"GEARBOX/res/icons/icons8-file-512-txt.png",		".txt"},
					{"GEARBOX/res/icons/icons8-file-512-user.png",		".user"},
					{"GEARBOX/res/icons/icons8-file-512-vcxproj.png",	".vcxproj"},
					{"GEARBOX/res/icons/icons8-file-512-wav.png",		".wav"},
					{"GEARBOX/res/icons/icons8-file-512-wma.png",		".wma"},
					{"GEARBOX/res/icons/icons8-file-512-wmv.png",		".wmv"},
					{"GEARBOX/res/icons/icons8-file-512-xml.png",		".xml"},
					{"GEARBOX/res/icons/icons8-file-512-yml.png",		".yml"},
					{"GEARBOX/res/icons/icons8-file-512-zip.png",		".zip"}
				};

			};
		}
	}
}