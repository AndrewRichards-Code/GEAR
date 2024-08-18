#pragma once

#include "Asset/Asset.h"
#include <filesystem>

namespace gear
{
	namespace asset
	{
		struct GEAR_API AssetMetadata
		{
			Asset::Type type = Asset::Type::NONE;
			std::filesystem::path filepath = std::filesystem::path("");
			std::filesystem::file_time_type lastWriteTime = std::filesystem::file_time_type();

			operator bool() const
			{
				return type != Asset::Type::NONE;
			}

			bool CheckFilepathExtension(const std::filesystem::path& extension) const
			{
				const std::filesystem::path& filepathExtension = filepath.extension();
				return filepathExtension == extension;
			}

			static bool IsFilepathExtensionImageType(const std::filesystem::path& extension)
			{
				static const std::vector<std::filesystem::path> ImageExtensions =
				{
					".png",
					".tga",
					".bmp",
					".jpeg",
					".jpg",
					".exr",
					".hdr",
				};
				return arc::FindInVector(ImageExtensions, extension);
			}
			bool IsFilepathExtensionImageType() const
			{
				return AssetMetadata::IsFilepathExtensionImageType(filepath.extension());
			}

			static bool IsFilepathExtensionFontType(const std::filesystem::path& extension)
			{
				static const std::vector<std::filesystem::path> FontExtensions = 
				{
					".ttf",
					".otf",
					".woff",
					".woff2",
				};
				return arc::FindInVector(FontExtensions, extension);
			}
			bool IsFilepathExtensionFontType() const
			{
				return AssetMetadata::IsFilepathExtensionFontType(filepath.extension());
			}
			static bool IsFilepathExtensionMeshType(const std::filesystem::path& extension)
			{
				static const std::vector<std::filesystem::path> MeshExtensions =
				{
					".fbx",
					".gltf",
				};
				return arc::FindInVector(MeshExtensions, extension);
			}
			bool IsFilepathExtensionMeshType() const
			{
				return AssetMetadata::IsFilepathExtensionMeshType(filepath.extension());
			}
		};
	}
}