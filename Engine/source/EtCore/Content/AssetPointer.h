#pragma once

#include "Asset.h"


//---------------------------------
// I_AssetPtr
//
// Base asset pointer is allowed to change the assets ref count
//
class I_AssetPtr
{
public:
	I_AssetPtr() = default; 
	I_AssetPtr(std::nullptr_t) {}

	I_AssetPtr& operator=(I_AssetPtr const& rhs);

	I_AssetPtr(I_Asset* asset);
	virtual ~I_AssetPtr();

	I_Asset const* GetAsset() const { return m_Asset; }
	inline std::type_info const& GetType() const;

	// utility
	bool is_null() const;
	void swap(I_AssetPtr& other) noexcept;

protected:
	// functions
	inline bool IncrementRefCount();
	inline bool DecrementRefCount();

	inline void Invalidate();

	// Data
	///////

	I_Asset* m_Asset = nullptr;
};


//---------------------------------
// AssetPtr
//
// Smart pointer that manages an assets lifetime by counting references. Doesn't delete the asset but loads or unloads it
//
template <class T_DataType>
class AssetPtr final : public I_AssetPtr
{
public:
	// default constructor initializes to null
	AssetPtr() : I_AssetPtr() {} 
	AssetPtr(std::nullptr_t) : I_AssetPtr() {} // implicityly convert nullptr to this type

	// Create from raw asset pointer
	explicit AssetPtr(RawAsset<T_DataType>* rawAsset);
	AssetPtr& operator=(RawAsset<T_DataType>* rawAsset);

	// copy asset pointers around
	AssetPtr(AssetPtr const& copy);
	AssetPtr& operator=(AssetPtr const& rhs);

	// Moving
	AssetPtr(AssetPtr&& moving);
	AssetPtr& operator=(AssetPtr&& moving);

	// deleting the pointer may cause unloading the asset if this was the last instance
	virtual ~AssetPtr() = default;

	// utility
	void swap(AssetPtr& other) noexcept;

public:
	// accessors
	// we can only retrieve const data from asset pointers
	T_DataType const* get() const; 
	T_DataType const* operator->() const;
	T_DataType const& operator*() const;
};


// Asset pointer comparisons
//--------------------------


bool operator == (I_AssetPtr const& ptr, std::nullptr_t);
bool operator == (std::nullptr_t, I_AssetPtr const& ptr);

template <class T_DataType>
bool operator == (AssetPtr<T_DataType> const& ptr, std::nullptr_t);

template <class T_DataType>
bool operator == (std::nullptr_t, AssetPtr<T_DataType> const& ptr);

template <class T_DataType>
bool operator == (AssetPtr<T_DataType> const& ptr1, AssetPtr<T_DataType> const& ptr2);

template <class T_DataType>
bool operator != (AssetPtr<T_DataType> const& ptr, std::nullptr_t);

template <class T_DataType>
bool operator != (std::nullptr_t, AssetPtr<T_DataType> const& ptr);

template <class T_DataType>
bool operator != (AssetPtr<T_DataType> const& ptr1, AssetPtr<T_DataType> const& ptr2);


#include "AssetPointer.inl"
