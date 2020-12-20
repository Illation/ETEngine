#include "stdafx.h"
#include "EditorAsset.h"

#include <EtCore/Content/ResourceManager.h>


namespace et {
namespace pl {


//---------------------------
// EditorAssetBase::Load
//
// Bypass runtime asset load but fullfill the same functionality
//
void EditorAssetBase::Load()
{
	for (EditorAssetBase* const child : m_ChildAssets)
	{
		child->GetAsset()->m_RefCount++; // manually increment refcount for hidden child assets
		child->Load();
	}

	core::I_Asset* const asset = GetAsset();

	// Make sure all references are loaded
	for (core::I_Asset::Reference& reference : asset->m_References)
	{
		reference.Ref();
	}

	// get binary data from the package
	if (!(core::ResourceManager::Instance()->GetLoadData(asset, asset->m_LoadData)))
	{
		ET_ASSERT(false, "Couldn't get data for '%s' (%i) in package '%s'",
			asset->GetPackageEntryId().ToStringDbg(),
			asset->GetPackageEntryId().Get(),
			asset->GetPackageId().ToStringDbg());
		return;
	}

	// let the asset load from binary data
	if (!LoadFromMemory(asset->m_LoadData))
	{
		ET_ASSERT(false, "Failed Loading editor asset from memory, id: '%s'", m_Id.ToStringDbg());
	}

	if (!(asset->m_IsPersistent))
	{
		asset->m_LoadData.clear();
	}

	// dereference non persistent references 
	for (core::I_Asset::Reference& reference : asset->m_References)
	{
		reference.Deref();
	}
}

//---------------------------
// EditorAssetBase::Unload
//
void EditorAssetBase::Unload(bool const force)
{
	core::ResourceManager const* const resMan = core::ResourceManager::Instance();

	if (force || resMan == nullptr || !(resMan->IsUnloadDeferred()))
	{
		UnloadInternal();

		for (EditorAssetBase* const child : m_ChildAssets)
		{
			child->Unload(force);
			child->GetAsset()->m_RefCount--; // since we manually incremented, we should also manually decrement
		}
	}
}


} // namespace pl
} // namespace et
