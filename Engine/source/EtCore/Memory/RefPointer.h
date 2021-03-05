#pragma once
#include "Create.h"
#include "RefCount.h"
#include "UniquePointer.h"


namespace et {


//--------
// RefPtr
//
// Pointer with shared ownership through ref count, last reference deletes data 
//  - unlike std::shared_ptr : can't be constructed from raw pointer. Doesn't use atomic operations to increment counter. Refcount not exposed
//
template <typename TDataType>
class RefPtr final
{
	template <typename>
	friend class RefPtr;

	template <typename>
	friend class WeakPtr;

	template <typename>
	friend class Ptr;

public:
	// default constructor initializes to null
	RefPtr() = default;

	// from nullptr
	RefPtr(std::nullptr_t) {} // implicityly convert nullptr to this type
	RefPtr& operator=(std::nullptr_t);

	// Construct from derived creation class
	template <typename TOtherType>
	RefPtr(Create<TOtherType>&& create);
	template <typename TOtherType>
	RefPtr& operator=(Create<TOtherType>&& create);

	// copy from other ref pointer
	RefPtr(RefPtr const& copy);
	RefPtr& operator=(RefPtr copy);

	template <typename TOtherType>
	RefPtr(RefPtr<TOtherType> const& copy);
	template <typename TOtherType>
	RefPtr& operator=(RefPtr<TOtherType> const& copy);

	// Moving from other refptr
	template <typename TOtherType>
	RefPtr(RefPtr<TOtherType>&& moving);
	template <typename TOtherType>
	RefPtr& operator=(RefPtr<TOtherType>&& moving);

	// Moving from unique pointer
	template <typename TOtherType>
	RefPtr(UniquePtr<TOtherType>&& unique);
	template <typename TOtherType>
	RefPtr& operator=(UniquePtr<TOtherType>&& unique);

	// deleting the pointer may cause unloading the unique if this was the last instance
	~RefPtr();

	// utility
	bool IsNull() const;
	void Swap(RefPtr& other) noexcept;

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


template <class TDataType>
bool operator == (RefPtr<TDataType> const& ptr, std::nullptr_t);

template <class TDataType>
bool operator == (std::nullptr_t, RefPtr<TDataType> const& ptr);

template <class TDataType>
bool operator == (RefPtr<TDataType> const& ptr1, RefPtr<TDataType> const& ptr2);

template <class TDataType>
bool operator != (RefPtr<TDataType> const& ptr, std::nullptr_t);

template <class TDataType>
bool operator != (std::nullptr_t, RefPtr<TDataType> const& ptr);

template <class TDataType>
bool operator != (RefPtr<TDataType> const& ptr1, RefPtr<TDataType> const& ptr2);


} // namespace et


#include "RefPointer.inl"
