#include "ExternalFileSerialiser.h"

#include "Asset/AssetFile.h"
#include "ExternalSerialiser/ImageSerialiser.h"
#include "ExternalSerialiser/ModelLoader.h"

#include "Objects/Mesh.h"

using namespace gear;
using namespace asset;
using namespace serialiser;

Ref<Asset> ExternalFileSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	if (metadata.IsFilepathExtensionImageType())
	{
		return ImageSerialiser::Deserialise(handle, metadata);
	}
	else if (metadata.IsFilepathExtensionMeshType())
	{
		return ModelLoader::Deserialise(handle, metadata);
	}
	else if (metadata.IsFilepathExtensionFontType())
	{
	}
	else
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::NOT_SUPPORTED, "Filepath extension: %s is not supported for Deserialisation", metadata.filepath.generic_string().c_str());
	}

	return CreateRef<Asset>();
}

void ExternalFileSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	if (metadata.IsFilepathExtensionImageType())
	{
		ImageSerialiser::Serialise(asset, metadata);
	}
	else
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::NOT_SUPPORTED, "Filepath extension: %s is not supported for Serialisation.", metadata.filepath.generic_string().c_str());
	}
}