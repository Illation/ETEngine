#pragma once
#include "UniquePointer.h"
#include "RefPointer.h"
#include "WeakPointer.h"


namespace et {


//-----------
// Ptr
//
// Simple wrapper around a raw pointer that is initialized to null by default, use for referencing data where the lifetime is known and managed externally
//
template <typename TDataType>
class Ptr final
{
	template <typename>
	friend class Ptr;

public:
	// static functionality
	static Ptr CreateFromRaw(TDataType* const raw);

	template <typename TOtherType>
	static Ptr StaticCast(Ptr<TOtherType> const& from);

	// default constructor initializes to null
	Ptr() = default;

	// from nullptr
	Ptr(std::nullptr_t) {} // implicityly convert nullptr to this type
	Ptr& operator=(std::nullptr_t);

	// Copy
	Ptr(Ptr const& copy) = default;
	Ptr& operator=(Ptr const& rhs) = default;

	template <typename TOtherType>
	Ptr(Ptr<TOtherType> const& copy);
	template <typename TOtherType>
	Ptr& operator=(Ptr<TOtherType> const& rhs);
	
	// from Unique
	template <typename TOtherType>
	Ptr(UniquePtr<TOtherType> const& unique);
	template <typename TOtherType>
	Ptr& operator=(UniquePtr<TOtherType> const& unique);

	// from Ref
	template <typename TOtherType>
	Ptr(RefPtr<TOtherType> const& ref);
	template <typename TOtherType>
	Ptr& operator=(RefPtr<TOtherType> const& ref);

	// from Weak
	template <typename TOtherType>
	Ptr(WeakPtr<TOtherType> const& weak);
	template <typename TOtherType>
	Ptr& operator=(WeakPtr<TOtherType> const& weak);

	// Moving
	Ptr(Ptr&& moving);
	Ptr& operator=(Ptr&& moving);

	template <typename TOtherType>
	Ptr(Ptr<TOtherType>&& moving);
	template <typename TOtherType>
	Ptr& operator=(Ptr<TOtherType>&& moving);

	// delete - nothing really happens
	~Ptr() = default;

	// utility
	bool IsNull() const;
	void Swap(Ptr& other) noexcept;

	// accessors
	TDataType* Get() const;
	TDataType* operator->() const;
	TDataType& operator*() const;

	// Data
	///////

private:
	TDataType* m_Ptr = nullptr;
};


// comparisons
//-------------


template <typename TDataType>
bool operator == (Ptr<TDataType> const& ptr, std::nullptr_t);

template <typename TDataType>
bool operator == (std::nullptr_t, Ptr<TDataType> const& ptr);

template <typename TDataType, typename TOtherType>
bool operator == (Ptr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (Ptr<TDataType> const& ptr1, UniquePtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (UniquePtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (Ptr<TDataType> const& ptr1, RefPtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (RefPtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (Ptr<TDataType> const& ptr1, WeakPtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (WeakPtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2);

template <typename TDataType>
bool operator != (Ptr<TDataType> const& ptr, std::nullptr_t);

template <typename TDataType>
bool operator != (std::nullptr_t, Ptr<TDataType> const& ptr);

template <typename TDataType, typename TOtherType>
bool operator != (Ptr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (Ptr<TDataType> const& ptr1, UniquePtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (UniquePtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (Ptr<TDataType> const& ptr1, RefPtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (RefPtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (Ptr<TDataType> const& ptr1, WeakPtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (WeakPtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2);


// other utility
//---------------


template <typename TDataType>
Ptr<TDataType> ToPtr(TDataType* const raw) { return Ptr<TDataType>::CreateFromRaw(raw); }


} // namespace et


#include "Pointer.inl"
