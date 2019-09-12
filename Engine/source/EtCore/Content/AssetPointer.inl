#pragma once


//=========================
// Abstract asset pointer
//=========================


//---------------------------------
// I_AssetPtr::GetType
//
// Underlying type
//
inline std::type_info const& I_AssetPtr::GetType() const
{
	if (m_Asset == nullptr)
	{
		return typeid(std::nullptr_t);
	}

	return m_Asset->GetType();
}

//---------------------------------
// I_AssetPtr::IncrementRefCount
//
// Increments the ref count and returns true if this is the first reference
//
inline bool I_AssetPtr::IncrementRefCount()
{
	m_Asset->m_RefCount++;
	return (m_Asset->m_RefCount == 1u);
}

//---------------------------------
// I_AssetPtr::DecrementRefCount
//
// Decrements the ref count and returns true if this was the last reference
//
inline bool I_AssetPtr::DecrementRefCount()
{
	ET_ASSERT(m_Asset->m_RefCount > 0u);
	m_Asset->m_RefCount--;
	return (m_Asset->m_RefCount == 0u);
}

//---------------------------------
// I_AssetPtr::Invalidate
//
// If this was a valid pointer, decrement the ref count and make sure the asset is null
//
inline void I_AssetPtr::Invalidate()
{
	if (m_Asset != nullptr)
	{
		if (DecrementRefCount())
		{
			ET_ASSERT(m_Asset->IsLoaded());
			m_Asset->Unload(); // Maybe we want to defer this to the resource manager
		}
		m_Asset = nullptr;
	}
}


//=========================
// Asset Pointer Template
//=========================


// From raw asset pointer
/////////////////////////

//---------------------------------
// AssetPtr::AssetPtr
//
// Creates a new pointer to this asset using the underlying I_AssetPtr implementation
//
template <class T_DataType>
AssetPtr<T_DataType>::AssetPtr(RawAsset<T_DataType>* rawAsset)
	: I_AssetPtr(static_cast<I_Asset*>(rawAsset))
{}

//---------------------------------
// AssetPtr::operator=
//
// Assign from a raw pointer to an asset. Decrements the ref count of the current asset and Increments the ref count of the new asset
//
template <class T_DataType>
AssetPtr<T_DataType>& AssetPtr<T_DataType>::operator=(RawAsset<T_DataType>* rawAsset)
{
	AssetPtr tmp(rawAsset); // asserts if raw asset is null
	tmp.swap(*this);
	return *this;
}


// From another AssetPtr
/////////////////////////

//---------------------------------
// AssetPtr::AssetPtr
//
// Copy construct from another asset pointer. The asset should now at least have two references
//
template <class T_DataType>
AssetPtr<T_DataType>::AssetPtr(AssetPtr<T_DataType> const& copy) 
	: I_AssetPtr(copy.m_Asset)
{ }

//---------------------------------
// AssetPtr::operator=
//
// Assign from another asset pointer. Will decrement the old assets pointer and increment the new assets pointer
//  - this works because the rhs AssetPtr is copied to tmp, the old data is swapped into the value which will then get deleted
//
template <class T_DataType>
AssetPtr<T_DataType>& AssetPtr<T_DataType>::operator=(AssetPtr<T_DataType> const& rhs)
{
	AssetPtr tmp(rhs);
	tmp.swap(*this);
	return *this;
}


// Move another AssetPtr
/////////////////////////

//---------------------------------
// AssetPtr::AssetPtr
//
// Move constructor - shouldn't increase the ref count. The old reference will be a nullptr since that's the default state of this pointer
//
template <class T_DataType>
AssetPtr<T_DataType>::AssetPtr(AssetPtr<T_DataType>&& moving)
{
	moving.swap(*this);
}

//---------------------------------
// AssetPtr::AssetPtr
//
// Move assignment - shouldn't increase the ref count. We make sure the old pointer is invalidated first.
//
template <class T_DataType>
AssetPtr<T_DataType>& AssetPtr<T_DataType>::operator=(AssetPtr<T_DataType>&& moving)
{
	Invalidate();
	moving.swap(*this);
	return *this;
}

// Utility
/////////////

//---------------------------------
// AssetPtr::swap
//
// Swap two asset pointers
//
template <class T_DataType>
void AssetPtr<T_DataType>::swap(AssetPtr<T_DataType>& other) noexcept
{
	std::swap(m_Asset, other.m_Asset);
}

//---------------------------------
// AssetPtr::is_null
//
// Cheap check if the pointer is null
//
template <class T_DataType>
bool AssetPtr<T_DataType>::is_null() const
{
	return (m_Asset == nullptr);
}


// Accessors
/////////////

//---------------------------------
// AssetPtr::get
//
// Get the content of the asset. The asset should be loaded by now
//
template <class T_DataType>
T_DataType const* AssetPtr<T_DataType>::get() const
{
	if (m_Asset != nullptr)
	{
		ET_ASSERT(m_Asset->IsLoaded());
		ET_ASSERT(m_Asset->GetType() == typeid(T_DataType));
		return static_cast<RawAsset<T_DataType>*>(m_Asset)->GetData();
	}

	return nullptr;
}

//---------------------------------
// AssetPtr::operator->
//
// Get the content of the asset using the pointer operator
//
template <class T_DataType>
T_DataType const* AssetPtr<T_DataType>::operator->() const
{
	ET_ASSERT(m_Asset != nullptr);
	return get();
}

//---------------------------------
// AssetPtr::operator*
//
// Dereference the asset pointers content
//
template <class T_DataType>
T_DataType const& AssetPtr<T_DataType>::operator*() const
{
	ET_ASSERT(m_Asset != nullptr);
	return *get();
}


//---------------------------------
// AssetPtr::operator==
//
// Compare pointer equals null
//
template <class T_DataType>
bool operator==(AssetPtr<T_DataType> const& ptr, std::nullptr_t)
{
	return ptr.is_null();
}

template <class T_DataType>
bool operator==(std::nullptr_t, AssetPtr<T_DataType> const& ptr)
{
	return ptr.is_null();
}

template <class T_DataType>
bool operator==(AssetPtr<T_DataType> const& ptr1, AssetPtr<T_DataType> const& ptr2)
{
	return (ptr1.get() == ptr2.get());
}

template <class T_DataType>
bool operator!=(AssetPtr<T_DataType> const& ptr, std::nullptr_t)
{
	return !ptr.is_null();
}

template <class T_DataType>
bool operator!=(std::nullptr_t, AssetPtr<T_DataType> const& ptr)
{
	return !ptr.is_null();
}

template <class T_DataType>
bool operator!=(AssetPtr<T_DataType> const& ptr1, AssetPtr<T_DataType> const& ptr2)
{
	return !(ptr1 == ptr2);
}
