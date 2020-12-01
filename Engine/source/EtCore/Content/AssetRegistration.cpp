#include "stdafx.h"
#include "AssetRegistration.h"


namespace et {
namespace core {


//---------------------------------
// IsAssetPointerType
//
bool IsAssetPointerType(rttr::type const type)
{
	rttr::variant val = type.get_metadata(GetHash("AssetPointerType"));
	return val.is_valid() && val.get_value<bool>();
}

//---------------------------------
// IsSerializableAssetPointerType
//
bool IsSerializableAssetPointerType(rttr::type const type)
{
	rttr::variant val = type.get_metadata(GetHash("AssetPointerType"));
	if (val.is_valid())
	{
		return val.get_value<bool>();
	}
	else
	{
		val = type.get_metadata(GetHash("AssetBaseType"));
		return val.is_valid() && val.get_value<bool>();
	}
}

//---------------------------------
// IsAssetDataType
//
bool IsAssetDataType(rttr::type const type)
{
	rttr::variant val = type.get_metadata(GetHash("AssetDataType"));
	return val.is_valid() && val.get_value<bool>();
}


} // namespace core
} // namespace et
