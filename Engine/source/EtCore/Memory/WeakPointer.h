#pragma once
#include "RefPointer.h"


namespace et {


//--------
// WeakPtr
//
// Created from a Refpointer without incrementing refcount. Can tell if the object instance is still live
//
template <typename TDataType>
class WeakPtr final
{
	template <typename>
	friend class RefPtr;

	template <typename>
	friend class WeakPtr;

	template <typename>
	friend class Ptr;

public:
	// static functionality
	template <typename TOtherType>
	static WeakPtr StaticCast(WeakPtr<TOtherType> const& from);

	// default constructor initializes to null
	WeakPtr() = default;

	// from nullptr
	WeakPtr(std::nullptr_t) {} // implicityly convert nullptr to this type
	WeakPtr& operator=(std::nullptr_t);

	// copy from other weak pointer
	WeakPtr(WeakPtr const& copy);
	WeakPtr& operator=(WeakPtr copy);

	template <typename TOtherType>
	WeakPtr(WeakPtr<TOtherType> const& copy);
	template <typename TOtherType>
	WeakPtr& operator=(WeakPtr<TOtherType> const& copy);

	// Moving from other weak pointer
	template <typename TOtherType>
	WeakPtr(WeakPtr<TOtherType>&& moving);
	template <typename TOtherType>
	WeakPtr& operator=(WeakPtr<TOtherType>&& moving);

	// Create from ref pointer
	template <typename TOtherType>
	WeakPtr(RefPtr<TOtherType> const& ref);
	template <typename TOtherType>
	WeakPtr& operator=(RefPtr<TOtherType> const& ref);

	// deleting the pointer may cause unloading the unique if this was the last instance
	~WeakPtr();

	// utility
	bool IsNull() const;
	void Swap(WeakPtr& other) noexcept;

	// accessors
	TDataType* Get() const;
	TDataType* operator->() const;
	TDataType& operator*() const;

	// private functionality
private:
	void StartRefCount();
	void DecrementRefcount();
	void Invalidate();

	// Data
	///////

	TDataType* m_Ptr = nullptr;
	RefCount* m_RefCount = nullptr;
};


// comparisons
//-------------


template <typename TDataType>
bool operator == (WeakPtr<TDataType> const& ptr, std::nullptr_t);

template <typename TDataType>
bool operator == (std::nullptr_t, WeakPtr<TDataType> const& ptr);

template <typename TDataType, typename TOtherType>
bool operator == (WeakPtr<TDataType> const& ptr1, WeakPtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (WeakPtr<TDataType> const& ptr1, RefPtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (RefPtr<TDataType> const& ptr1, WeakPtr<TOtherType> const& ptr2);

template <typename TDataType>
bool operator != (WeakPtr<TDataType> const& ptr, std::nullptr_t);

template <typename TDataType>
bool operator != (std::nullptr_t, WeakPtr<TDataType> const& ptr);

template <typename TDataType, typename TOtherType>
bool operator != (WeakPtr<TDataType> const& ptr1, WeakPtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (WeakPtr<TDataType> const& ptr1, RefPtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (RefPtr<TDataType> const& ptr1, WeakPtr<TOtherType> const& ptr2);


} // namespace et


#include "WeakPointer.inl"
