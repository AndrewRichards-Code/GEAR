#pragma once
#include "Asset/AssetMetadata.h"
#include "Asset/AssetDataBuffer.h"
#include "Asset/Serialiser/AssetSerialiser.h"
#include "Objects/Font.h"

typedef struct FT_LibraryRec_* FT_Library;

namespace gear
{
	namespace asset
	{
		namespace serialiser
		{
			class GEAR_ASSET_MANAGER_API FontSerialiser
			{
				//Methods
			public:
				static Ref<Asset> Deserialise(Asset::Handle handle, const AssetMetadata& metadata);
				static void Serialise(Ref<Asset> asset, const AssetMetadata& metadata);

			private:
				FontSerialiser();
				~FontSerialiser();

				static Ref<objects::Font> LoadGeneratedFont(const AssetMetadata& metadata);
				static void SaveGeneratedFont(const Ref<asset::ImageAssetDataBuffer> dataBuffer, const std::array<objects::Font::GlyphInfo, objects::Font::s_NumCaracters>& glyphInfos, const AssetMetadata& metadata);

				static Ref<objects::Font> GenerateFont(const AssetMetadata& metadata);

				//Members
			private:
				
				static FT_Library m_FT_Lib;
			};
		}
	}
}