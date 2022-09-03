#pragma once
#include <rttr/wrapper_mapper.h>

#include "Create.h"


namespace et {


//-----------
// UniquePtr
//
// Pointer that ensures no other references manage it's lifetime - unlike std::unique_ptr it can't be constructed from a raw pointer
//
template <typename TDataType>
class UniquePtr final 
{
	template <typename>
	friend class UniquePtr;

	template <typename>
	friend class Ptr;

public:
	// static functionality
	template <typename TOtherType>
	static UniquePtr StaticCast(UniquePtr<TOtherType>&& from);

	// default constructor initializes to null
	UniquePtr() = default;

	// from nullptr
	UniquePtr(std::nullptr_t) {} // implicityly convert nullptr to this type
	UniquePtr& operator=(std::nullptr_t);

	// Construct from derived creation class
	template <typename TOtherType>
	UniquePtr(Create<TOtherType>&& create);
	template <typename TOtherType>
	UniquePtr& operator=(Create<TOtherType>&& create);

	// can't copy unique pointers around
	UniquePtr(UniquePtr const& copy) = delete;
	UniquePtr& operator=(UniquePtr const& rhs) = delete;

	// Moving
	UniquePtr(UniquePtr&& moving);
	UniquePtr& operator=(UniquePtr&& moving);

	template <typename TOtherType>
	UniquePtr(UniquePtr<TOtherType>&& moving);
	template <typename TOtherType>
	UniquePtr& operator=(UniquePtr<TOtherType>&& moving);

	// deleting the pointer may cause unloading the unique if this was the last instance
	~UniquePtr();

	// utility
	bool IsNull() const;
	void Swap(UniquePtr& other) noexcept;
	TDataType* Release();
	void Reset();

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
bool operator == (UniquePtr<TDataType> const& ptr, std::nullptr_t);

template <typename TDataType>
bool operator == (std::nullptr_t, UniquePtr<TDataType> const& ptr);

template <typename TDataType, typename TOtherType>
bool operator == (UniquePtr<TDataType> const& ptr1, UniquePtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (TDataType* const ptr1, UniquePtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator == (UniquePtr<TDataType> const& ptr1, TOtherType const* const ptr2);

template <typename TDataType>
bool operator != (UniquePtr<TDataType> const& ptr, std::nullptr_t);

template <typename TDataType>
bool operator != (std::nullptr_t, UniquePtr<TDataType> const& ptr);

template <typename TDataType, typename TOtherType>
bool operator != (UniquePtr<TDataType> const& ptr1, UniquePtr<TOtherType> const& ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (UniquePtr<TDataType> const& ptr1, TOtherType* const ptr2);

template <typename TDataType, typename TOtherType>
bool operator != (TDataType* const ptr1, UniquePtr<TOtherType> const& ptr2);


} // namespace et


#include "UniquePointer.inl"
