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
// I_AssetPtr::operator=
//
// Copy construct the base asset ptr
//
I_AssetPtr::I_AssetPtr(I_AssetPtr const& copy)
	: m_Asset(copy.m_Asset)
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
// I_AssetPtr::operator=
//
// Copy the base asset ptr
//
I_AssetPtr& I_AssetPtr::operator=(I_AssetPtr const& rhs)
{
	I_AssetPtr tmp(rhs.m_Asset);
	tmp.swap(*this);
	return *this;
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

//---------------------------------
// I_AssetPtr::is_null
//
// Cheap check if the pointer is null
//
bool I_AssetPtr::is_null() const
{
	return (m_Asset == nullptr);
}

//---------------------------------
// I_AssetPtr::swap
//
// Swap two asset pointers
//
void I_AssetPtr::swap(I_AssetPtr& other) noexcept
{
	ET_ASSERT(m_Asset == nullptr || other.m_Asset == nullptr || GetType() == other.GetType());
	std::swap(m_Asset, other.m_Asset);
}

//---------------------------------
// I_AssetPtr::operator==
//
bool operator==(I_AssetPtr const& ptr, std::nullptr_t)
{
	return ptr.is_null();
}

//---------------------------------
// I_AssetPtr::operator==
//
bool operator==(std::nullptr_t, I_AssetPtr const& ptr)
{
	return ptr.is_null();
}
