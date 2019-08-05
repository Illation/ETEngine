#pragma once

#include "Asset.h"

//---------------------------------
// I_AssetPtr
//
// Base asset pointer is allowed to change the assets ref count
//
class I_AssetPtr
{
protected:
	bool IncrementRefCount() 
	{ 
		m_Asset->m_RefCount++; 
		return (m_Asset->m_RefCount == 1u);
	}
	bool DecrementRefCount() 
	{ 
		ET_ASSERT(m_Asset->m_RefCount > 0u);
		m_Asset->m_RefCount--; 
		return (m_Asset->m_RefCount == 0u);
	}

	I_Asset* m_Asset;
};

//---------------------------------
// AssetPtr
//
// Smart pointer that manages an assets lifetime
//
template <class T_DataType>
class AssetPtr final : public I_AssetPtr
{
	explicit AssetPtr(I_Asset* rawAsset)
		: m_Asset(rawAsset)
	{
		ET_ASSERT(rawAsset != nullptr);
		if (IncrementRefCount())
		{
			ET_ASSERT(!m_Asset->IsLoaded()));
			m_Asset->Load();
		}
	}
	~AssetPtr()
	{
		if (DecrementRefCount())
		{
			ET_ASSERT(m_Asset->IsLoaded());
			m_Asset->Unload(); // Maybe we want to defer this to the resource manager
		}
	}
	AssetPtr(AssetPtr const& copy)
		: m_Asset(copy.m_Asset)
	{
		ET_ASSERT(m_Asset->GetRefCount() > 0u);
		IncrementRefCount();
	}
	AssetPtr& operator=(AssetPtr rhs)
	{
		rhs.swap(*this);
		return *this;
	}
	AssetPtr& operator=(I_Asset* rawAsset)
	{
		AssetPtr tmp(rawAsset); // asserts if raw asset is null
		tmp.swap(*this);
		return *this;
	}
	void swap(AssetPtr& other) noexcept
	{
		std::swap(m_Asset, other.m_Asset);
	}
	T_DataType* get() const 
	{ 
		ET_ASSERT(m_Asset->IsLoaded());
		return static_cast<RawAsset<T_DataType>*>(m_Asset)->GetData(); 
	}
	T_DataType* operator->() const { return get(); }
	T_DataType& operator*() const { return *get(); }
};
