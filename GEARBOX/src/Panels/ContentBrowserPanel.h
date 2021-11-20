#pragma once
#include "UIContext.h"

namespace gearbox
{
	namespace panels
	{
		class ContentBrowserPanel final : public Panel
		{
			//enums/structs
		public:
			struct CreateInfo
			{
				std::filesystem::path currentPath;
			};

			//Methods
		public:
			ContentBrowserPanel(CreateInfo* pCreateInfo);
			~ContentBrowserPanel();

			void Draw() override;

		public:
			inline CreateInfo& GetCreateInfo() { return m_CI; }

			//Members
		private:
			CreateInfo m_CI;
			
			Ref<gear::graphics::Texture> m_FolderTexture;
			Ref<gear::graphics::Texture> m_FileTexture;
			const std::string m_FolderTextureFilepath = "res/icons/icons8-folder-512.png";
			const std::string m_FileTextureFilepath = "res/icons/icons8-file-512.png";
			ImTextureID m_FolderTextureID = 0;
			ImTextureID m_FileTextureID = 0;

			struct FileExtTexture
			{
				std::string						filename;
				std::string						extension;
				Ref<gear::graphics::Texture>	texture = nullptr;
				ImTextureID						id = 0;
			};
			std::vector<FileExtTexture> m_FileExtTextureFilepath =
			{
				{"res/icons/icons8-file-512-a.png",			".a"},
				{"res/icons/icons8-file-512-aif.png",		".aif"},
				{"res/icons/icons8-file-512-avi.png",		".avi"},
				{"res/icons/icons8-file-512-bak.png",		".bak"},
				{"res/icons/icons8-file-512-bat.png",		".bat"},
				{"res/icons/icons8-file-512-bin.png",		".bin"},
				{"res/icons/icons8-file-512-c.png",			".c"},
				{"res/icons/icons8-file-512-cmake.png",		".cmake"},
				{"res/icons/icons8-file-512-cpp.png",		".cpp"},
				{"res/icons/icons8-file-512-cs.png",		".cs"},
				{"res/icons/icons8-file-512-cso.png",		".cso"},
				{"res/icons/icons8-file-512-cxx.png",		".cxx"},
				{"res/icons/icons8-file-512-dll.png",		".dll"},
				{"res/icons/icons8-file-512-dmp.png",		".dmp"},
				{"res/icons/icons8-file-512-exe.png",		".exe"},
				{"res/icons/icons8-file-512-exr.png",		".exr"},
				{"res/icons/icons8-file-512-exp.png",		".exp"},
				{"res/icons/icons8-file-512-fbx.png",		".fbx"},
				{"res/icons/icons8-file-512-filters.png",	".filters"},
				{"res/icons/icons8-file-512-h.png",			".h"},
				{"res/icons/icons8-file-512-hdr.png",		".hdr"},
				{"res/icons/icons8-file-512-hlsl.png",		".hlsl"},
				{"res/icons/icons8-file-512-hpp.png",		".hpp"},
				{"res/icons/icons8-file-512-html.png",		".html"},
				{"res/icons/icons8-file-512-icf.png",		".icf"},
				{"res/icons/icons8-file-512-ico.png",		".ico"},
				{"res/icons/icons8-file-512-idb.png",		".idb"},
				{"res/icons/icons8-file-512-ilk.png",		".ilk"},
				{"res/icons/icons8-file-512-ini.png",		".ini"},
				{"res/icons/icons8-file-512-inl.png",		".inl"},
				{"res/icons/icons8-file-512-iobj.png",		".iobj"},
				{"res/icons/icons8-file-512-ipdb.png",		".ipdb"},
				{"res/icons/icons8-file-512-iso.png",		".iso"},
				{"res/icons/icons8-file-512-ixx.png",		".ixx"},
				{"res/icons/icons8-file-512-java.png",		".java"},
				{"res/icons/icons8-file-512-jpg.png",		".jpg"},
				{"res/icons/icons8-file-512-json.png",		".json"},
				{"res/icons/icons8-file-512-lib.png",		".lib"},
				{"res/icons/icons8-file-512-lnk.png",		".lnk"},
				{"res/icons/icons8-file-512-log.png",		".log"},
				{"res/icons/icons8-file-512-md.png",		".md"},
				{"res/icons/icons8-file-512-midi.png",		".midi"},
				{"res/icons/icons8-file-512-mp3.png",		".mp3"},
				{"res/icons/icons8-file-512-mp4.png",		".mp4"},
				{"res/icons/icons8-file-512-o.png",			".o"},
				{"res/icons/icons8-file-512-obj.png",		".obj"},
				{"res/icons/icons8-file-512-ogg.png",		".ogg"},
				{"res/icons/icons8-file-512-pch.png",		".pch"},
				{"res/icons/icons8-file-512-pdb.png",		".pdb"},
				{"res/icons/icons8-file-512-pdf.png",		".pdf"},
				{"res/icons/icons8-file-512-png.png",		".png"},
				{"res/icons/icons8-file-512-psd.png",		".psd"},
				{"res/icons/icons8-file-512-py.png",		".py"},
				{"res/icons/icons8-file-512-rar.png",		".rar"},
				{"res/icons/icons8-file-512-sh.png",		".sh"},
				{"res/icons/icons8-file-512-sln.png",		".sln"},
				{"res/icons/icons8-file-512-spv.png",		".spv"},
				{"res/icons/icons8-file-512-svg.png",		".svg"},
				{"res/icons/icons8-file-512-tga.png",		".tga"},
				{"res/icons/icons8-file-512-ttf.png",		".ttf"},
				{"res/icons/icons8-file-512-txt.png",		".txt"},
				{"res/icons/icons8-file-512-user.png",		".user"},
				{"res/icons/icons8-file-512-vcxproj.png",	".vcxproj"},
				{"res/icons/icons8-file-512-wav.png",		".wav"},
				{"res/icons/icons8-file-512-wma.png",		".wma"},
				{"res/icons/icons8-file-512-wmv.png",		".wmv"},
				{"res/icons/icons8-file-512-xml.png",		".xml"},
				{"res/icons/icons8-file-512-yml.png",		".yml"},
				{"res/icons/icons8-file-512-zip.png",		".zip"}
			};

		};
	}
}