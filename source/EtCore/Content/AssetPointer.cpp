#include "stdafx.h"
#include "AssetPointer.h"


//=========================
// Abstract asset pointer
//=========================


//---------------------------------
// I_AssetPtr::c-tor
//
// Creates a new pointer to this asset. If the first reference was created, the asset is loaded
//
I_AssetPtr::I_AssetPtr(I_Asset* asset)
	: m_Asset(asset)
{
	if (m_Asset != nullptr) // having asset pointers point to null is valid
	{
		if (IncrementRefCount())
		{
			ET_ASSERT(!(m_Asset->IsLoaded()));
			m_Asset->Load();
		}
	}
}

//---------------------------------
// I_AssetPtr::d-tor
//
// Deletes the pointer. If this was the last reference the asset is unloaded
//
I_AssetPtr::~I_AssetPtr()
{
	Invalidate();
}

