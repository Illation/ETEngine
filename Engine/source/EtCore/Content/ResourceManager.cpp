#include "stdafx.h"
#include "ResourceManager.h"

#include "AssetDatabase.h"


namespace et {
namespace core {


//===================
// Resource Manager
//===================


// static
UniquePtr<ResourceManager> ResourceManager::s_Instance;


//----------------------------------
// ResourceManager::SetInstance
//
// Initializes the singleton with a derived implementation
//
void ResourceManager::SetInstance(UniquePtr<ResourceManager>&& instance)
{
	s_Instance = std::move(instance);
	s_Instance->Init();
}

//----------------------------------
// ResourceManager::DestroyInstance
//
// Deinitializes the singleton
//
void ResourceManager::DestroyInstance()
{
	s_Instance = nullptr;
}

//-------------------------------------
// ResourceManager::SetAssetReferences
//
// Runs the setter function on each asset reference in the DB
//
void ResourceManager::SetAssetReferences(I_AssetDatabase* const db, T_ReferenceAssetGetter const& fnc) const
{
	db->IterateAllAssets([fnc](I_Asset* const asset)
		{
			for (I_Asset::Reference& reference : asset->m_References)
			{
				reference.m_Asset = fnc(reference.m_Id);
			}
		});
}

//-----------------------------
// ResourceManager::LoadAsset
//
void ResourceManager::LoadAsset(I_Asset* const asset)
{
	ET_ASSERT(!(asset->IsLoaded()));
	asset->Load();
}

//-------------------------------
// ResourceManager::UnloadAsset
//
void ResourceManager::UnloadAsset(I_Asset* const asset)
{
	ET_ASSERT(asset->IsLoaded());
	asset->Unload();
}


} // namespace core
} // namespace et
