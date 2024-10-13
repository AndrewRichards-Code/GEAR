#include "AssetSerialiser.h"

#include "Asset/Serialiser/AudioSerialiser.h"
#include "Asset/Serialiser/ExternalFileSerialiser.h"
#include "Asset/Serialiser/FontSerialiser.h"
#include "Asset/Serialiser/MaterialSerialiser.h"
#include "Asset/Serialiser/MeshSerialiser.h"
#include "Asset/Serialiser/SceneSerialiser.h"
#include "Asset/Serialiser/TextureSerialiser.h"

#include "Graphics/Window.h"
#include "Project/Project.h"

using namespace gear;
using namespace asset;
using namespace serialiser;

void AssetSerialiser::SetDevice(void* device)
{
	s_Device = device;
}

void* AssetSerialiser::GetDevice()
{ 
	return s_Device;
}

std::map<Asset::Type, AssetSerialiser::AssetDeserialiseFunction> AssetSerialiser::s_AssetDeserialiseFunctions = {
	{ Asset::Type::AUDIO,			AudioSerialiser::Deserialise },
	{ Asset::Type::EXTERNAL_FILE,	ExternalFileSerialiser::Deserialise },
	{ Asset::Type::FONT,			FontSerialiser::Deserialise },
	{ Asset::Type::MATERIAL,		MaterialSerialiser::Deserialise },
	{ Asset::Type::MESH,			MeshSerialiser::Deserialise },
	{ Asset::Type::SCENE,			SceneSerialiser::Deserialise },
	{ Asset::Type::TEXTURE,			TextureSerialiser::Deserialise },
};

std::map<Asset::Type, AssetSerialiser::AssetSerialiseFunctions> AssetSerialiser::s_AssetSerialiseFunctions = {
	{ Asset::Type::AUDIO,			AudioSerialiser::Serialise },
	{ Asset::Type::EXTERNAL_FILE,	ExternalFileSerialiser::Serialise },
	{ Asset::Type::FONT,			FontSerialiser::Serialise },
	{ Asset::Type::MATERIAL,		MaterialSerialiser::Serialise },
	{ Asset::Type::MESH,			MeshSerialiser::Serialise },
	{ Asset::Type::SCENE,			SceneSerialiser::Serialise },
	{ Asset::Type::TEXTURE,			TextureSerialiser::Serialise },
};

Ref<Asset> AssetSerialiser::Deserialise(Asset::Handle handle, const AssetMetadata& metadata)
{
	if (s_AssetDeserialiseFunctions.find(metadata.type) == s_AssetDeserialiseFunctions.end())
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::INVALID_VALUE, "No Deserialiser for Asset::Type::%s", Asset::ToString(metadata.type).c_str());
		return nullptr;
	}
	return s_AssetDeserialiseFunctions.at(metadata.type)(handle, metadata);
}

void AssetSerialiser::Serialise(Ref<Asset> asset, const AssetMetadata& metadata)
{
	if (s_AssetSerialiseFunctions.find(metadata.type) == s_AssetSerialiseFunctions.end())
	{
		GEAR_WARN(ErrorCode::ASSET | ErrorCode::INVALID_VALUE, "No Serialiser for Asset::Type::%s", Asset::ToString(metadata.type).c_str());
		return;
	}
	s_AssetSerialiseFunctions.at(metadata.type)(asset, metadata);
}