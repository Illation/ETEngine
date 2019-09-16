#pragma once


//===================
// Resource Manager
//===================


//---------------------------------
// ResourceManager::GetAssetData
//
// Get the data of an asset. Loads the asset if it's not loaded yet
//
template <class T_DataType>
AssetPtr<T_DataType> ResourceManager::GetAssetData(T_Hash const assetId)
{
	// Get the asset
	RawAsset<T_DataType>* asset = static_cast<RawAsset<T_DataType>*>(GetAssetInternal(assetId, typeid(T_DataType)));

	// Check we actually found the asset
	if (asset == nullptr)
	{
		LOG("ResourceManager::GetAssetData > Couldn't find asset with ID '" + std::to_string(assetId) + std::string("'!"), LogLevel::Warning);
		return nullptr;
	}

	// Make sure to not unload non persistent references immediately in case they appear multiple times in the hierachy
	m_DeferUnloadToFlush = true;

	AssetPtr<T_DataType> retPtr(asset); // this should load the data if it is accessed the first time

	m_DeferUnloadToFlush = false;

	// later if we load levels in batches we should also defer flushing to the end of the batch load
	Flush(); // as far as we know there won't be any more accesses to references during load so we should make sure to free the memory we won't need

	return retPtr;
}
