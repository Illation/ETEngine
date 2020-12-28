#include "stdafx.h"
#include "EditorAsset.h"

#include <EtCore/Content/ResourceManager.h>


namespace et {
namespace pl {


RTTR_REGISTRATION
{
	rttr::registration::class_<EditorAssetBase>("editor asset")
		.property("asset", &EditorAssetBase::m_Asset)
		.property("metadata", &EditorAssetBase::m_MetaData)
		.property("children", &EditorAssetBase::m_ChildAssets);
}


//=====================================
// Editor Asset :: Runtime Asset Data
//=====================================


//------------------------------------------
// EditorAssetBase::RuntimeAssetData::c-tor
//
EditorAssetBase::RuntimeAssetData::RuntimeAssetData(core::I_Asset* const asset, bool const ownsAsset) 
	: m_Asset(asset)
	, m_OwnsAsset(ownsAsset)
{ }

//------------------------------------------
// EditorAssetBase::RuntimeAssetData::d-tor
//
EditorAssetBase::RuntimeAssetData::~RuntimeAssetData()
{
	if (m_OwnsAsset)
	{
		delete m_Asset;
	}
}


//===============
// Editor Asset
//===============


//---------------------------
// EditorAssetBase::d-tor
//
EditorAssetBase::~EditorAssetBase()
{
	for (EditorAssetBase* const child : m_ChildAssets)
	{
		delete child;
	}

	Unload();

	delete m_MetaData;
}

//-----------------------
// EditorAssetBase::Init
//
void EditorAssetBase::Init(core::File* const configFile)
{
	m_File = configFile;
	m_Id = m_Asset->GetId();
}

//--------------------------------------
// EditorAssetBase::GetAllRuntimeAssets
//
// Recursively access runtime assets
//	
std::vector<EditorAssetBase::RuntimeAssetInfo> EditorAssetBase::GetAllRuntimeAssets() const
{
	ET_ASSERT(m_HasRuntimeAssets);

	std::vector<RuntimeAssetInfo> ret;

	for (EditorAssetBase const* const child : m_ChildAssets)
	{
		std::vector<RuntimeAssetInfo> childRet = child->GetAllRuntimeAssets();
		ret.insert(ret.end(), childRet.begin(), childRet.end());
	}

	for (RuntimeAssetData const& data : m_RuntimeAssets)
	{
		ret.emplace_back(data);
	}

	return ret;
}

//---------------------------------------------
// EditorAssetBase::SetupRuntimeAssetsInternal
//
// Default behavior just uses the same asset as the editor
//	
void EditorAssetBase::SetupRuntimeAssetsInternal()
{
	m_RuntimeAssets.emplace_back(m_Asset, false);
}

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

//-------------------------------------
// EditorAssetBase::SetupRuntimeAssets
//
// Populate the list of assets that the runtime should be using, with correct IDs and references. Doesn't need to generate the load data
//
void EditorAssetBase::SetupRuntimeAssets()
{
	ET_ASSERT(!m_HasRuntimeAssets);

	for (EditorAssetBase* const child : m_ChildAssets)
	{
		child->SetupRuntimeAssets();
	}

	SetupRuntimeAssetsInternal();
	m_HasRuntimeAssets = true;
}


} // namespace pl
} // namespace et
