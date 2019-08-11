#pragma once


//===================
// Resource Manager
//===================


//---------------------------------
// ResourceManager::GetAsset
//
// Get an asset by it's template type
//
template <class T_DataType>
RawAsset<T_DataType>* ResourceManager::GetAsset(T_Hash const assetId)
{
	I_Asset* abstractAsset = m_Database.GetAsset(assetId, typeid(T_DataType));
	return static_cast<RawAsset<T_DataType>*>(abstractAsset);
}


//---------------------------------
// ResourceManager::GetAssetData
//
// Get the data of an asset. Loads the asset if it's not loaded yet
//
template <class T_DataType>
AssetPtr<T_DataType> ResourceManager::GetAssetData(T_Hash const assetId)
{
	// Get the asset
	RawAsset<T_DataType>* asset = GetAsset<T_DataType>(assetId);

	// Check we actually found the asset
	if (asset == nullptr)
	{
		LOG("ResourceManager::GetAssetData > Couldn't find asset with ID '" + std::to_string(assetId) + std::string("'!"), LogLevel::Warning);
		return nullptr;
	}

	return AssetPtr<T_DataType>(asset); // this should load the data if it is accessed the first time
}

