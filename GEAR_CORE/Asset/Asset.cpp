#include "Asset.h"
#include "ARC/External/magic_enum/magic_enum.hpp"

using namespace gear::asset;

std::string Asset::ToString(Type type)
{
	return std::string(magic_enum::enum_name<Type>(type));
}

Asset::Type Asset::FromString(const std::string& assetType)
{
	//Find Type or return Type::NONE
	return magic_enum::enum_cast<Type>(std::string_view(assetType)).value_or(Type::NONE);
}
