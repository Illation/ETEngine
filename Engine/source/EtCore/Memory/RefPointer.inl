#pragma once


namespace et {


//=============
// Ref Pointer
//=============


// From nullptr
////////////////

//------------------
// RefPtr::c-tor
//
template <typename TDataType>
RefPtr<TDataType>& RefPtr<TDataType>::operator=(std::nullptr_t)
{
	Invalidate();
	return *this;
}


// From creator
////////////////

//------------------
// RefPtr::c-tor
//
// Move data in from creation and invalidate creation class. This creates our first reference
//
template <typename TDataType>
template <typename TOtherType>
RefPtr<TDataType>::RefPtr(Create<TOtherType>&& create)
{
	TDataType* tempPtr = create.Release();
	std::swap(m_Ptr, tempPtr);
	StartRefCount();
}

//---------------------------------
// RefPtr::operator=
//
// Move data in from creation and invalidate creation class. This creates our first reference
//
template <typename TDataType>
template <typename TOtherType>
RefPtr<TDataType>& RefPtr<TDataType>::operator=(Create<TOtherType>&& create)
{
	DecrementRefcount();

	TDataType* tempPtr = create.Release();
	std::swap(m_Ptr, tempPtr);

	StartRefCount();

	return *this;
}


// copy from other reference
/////////////////////////////

//---------------------------------
// RefPtr::c-tor
//
// copy and increment the refcount
//
template <typename TDataType>
RefPtr<TDataType>::RefPtr(RefPtr<TDataType> const& copy)
	: m_Ptr(copy.m_Ptr)
	, m_RefCount(copy.m_RefCount)
{
	if (m_RefCount != nullptr)
	{
		++(m_RefCount->m_References);
	}
}

//---------------------------------
// RefPtr::operator=
//
// Copy assignment - increment refcount - using copy and swap idiom
//
template <typename TDataType>
RefPtr<TDataType>& RefPtr<TDataType>::operator=(RefPtr<TDataType> copy)
{
	copy.Swap(*this);
	return *this;
}

//---------------------------------
// RefPtr::c-tor
//
// copy and increment the refcount
// Version supporting derived classes
//
template <typename TDataType>
template <typename TOtherType>
RefPtr<TDataType>::RefPtr(RefPtr<TOtherType> const& copy)
	: m_RefCount(copy.m_RefCount)
{
	TDataType* tempPtr = copy.m_Ptr;
	std::swap(m_Ptr, tempPtr);
	if (m_RefCount != nullptr)
	{
		++(m_RefCount->m_References);
	}
}

//---------------------------------
// RefPtr::operator=
//
// Copy assignment - increment refcount
// Version supporting derived classes
//
template <typename TDataType>
template <typename TOtherType>
RefPtr<TDataType>& RefPtr<TDataType>::operator=(RefPtr<TOtherType> const& copy)
{
	TDataType* tempPtr = copy.m_Ptr;
	std::swap(m_Ptr, tempPtr);

	m_RefCount = copy.m_RefCount;
	if (m_RefCount != nullptr)
	{
		++(m_RefCount->m_References);
	}

	return *this;
}


// move from refptr
////////////////////

//---------------------------------
// RefPtr::c-tor
//
// Move constructor - invalidate old pointer
//
template <typename TDataType>
template <typename TOtherType>
RefPtr<TDataType>::RefPtr(RefPtr<TOtherType>&& moving)
{
	// copy data. moved refcount stays intact, our own refcount will be 0 anyway
	TDataType* tempPtr = moving.m_Ptr;
	std::swap(m_Ptr, tempPtr);
	std::swap(m_RefCount, moving.m_RefCount);

	// invalidate old pointer
	moving.m_Ptr = nullptr;
}

//---------------------------------
// RefPtr::operator=
//
// Move assignment - invalidate old pointer
//
template <typename TDataType>
template <typename TOtherType>
RefPtr<TDataType>& RefPtr<TDataType>::operator=(RefPtr<TOtherType>&& moving)
{
	// if we hold a reference remove it
	DecrementRefcount();

	// copy data. moved refcount stays intact
	TDataType* tempPtr = moving.m_Ptr;
	std::swap(m_Ptr, tempPtr);
	m_RefCount = copy.m_RefCount;

	// invalidate old pointer
	moving.m_Ptr = nullptr;
	moving.m_RefCount = nullptr;

	return *this;
}


// move from unique ptr
////////////////////////

//---------------------------------
// RefPtr::c-tor
//
// Move constructor from unique pointer
//
template <typename TDataType>
template <typename TOtherType>
RefPtr<TDataType>::RefPtr(UniquePtr<TOtherType>&& unique)
{
	// copy data. moved refcount stays intact, our own refcount will be 0 anyway
	TDataType* tempPtr = unique.Release();
	std::swap(m_Ptr, tempPtr);

	StartRefCount();
}

//---------------------------------
// RefPtr::operator=
//
// Move assignment from unique pointer
//
template <typename TDataType>
template <typename TOtherType>
RefPtr<TDataType>& RefPtr<TDataType>::operator=(UniquePtr<TOtherType>&& unique)
{
	// if we hold a reference remove it
	DecrementRefcount();

	// copy data. moved refcount stays intact
	TDataType* tempPtr = unique.Release();
	std::swap(m_Ptr, tempPtr);

	StartRefCount();

	return *this;
}


// destroy
/////////////

//---------------------------------
// RefPtr::d-tor
//
// decrement ref count, delete data if there are no reference, delete refcount if there aren't even any weak references
//
template <typename TDataType>
RefPtr<TDataType>::~RefPtr()
{
	DecrementRefcount();
}


// Utility
/////////////

//---------------------------------
// RefPtr::IsNull
//
template <typename TDataType>
bool RefPtr<TDataType>::IsNull() const
{
	return (m_Ptr == nullptr);
}

//---------------------------------
// RefPtr::Swap
//
// Swap two unique pointers
//
template <typename TDataType>
void RefPtr<TDataType>::Swap(RefPtr<TDataType>& other) noexcept
{
	std::swap(m_Ptr, other.m_Ptr);
	std::swap(m_RefCount, other.m_RefCount);
}


// Accessors
/////////////

//---------------------------------
// RefPtr::Get
//
template <typename TDataType>
TDataType* RefPtr<TDataType>::Get() const
{
	return m_Ptr;
}

//---------------------------------
// RefPtr::operator->
//
template <typename TDataType>
TDataType* RefPtr<TDataType>::operator->() const
{
	ET_ASSERT(m_Ptr != nullptr);
	return Get();
}

//---------------------------------
// RefPtr::operator*
//
template <typename TDataType>
TDataType& RefPtr<TDataType>::operator*() const
{
	ET_ASSERT(m_Ptr != nullptr);
	return *Get();
}


// private
///////////

//---------------------------------
// RefPtr::StartRefCount
//
template <typename TDataType>
void et::RefPtr<TDataType>::StartRefCount()
{
	if (m_Ptr != nullptr)
	{
		m_RefCount = new RefCount();
	}
}

//---------------------------------
// RefPtr::DecrementRefcount
//
template <typename TDataType>
void RefPtr<TDataType>::DecrementRefcount()
{
	if (m_Ptr != nullptr)
	{
		--(m_RefCount->m_References);
		if (m_RefCount->m_References == 0u)
		{
			delete m_Ptr;

			if (m_RefCount->m_WeakReferences == 0u)
			{
				delete m_RefCount;
			}
		}
	}
}

//---------------------------------
// RefPtr::DecrementRefcount
//
template <typename TDataType>
void RefPtr<TDataType>::Invalidate()
{
	DecrementRefcount();
	m_Ptr = nullptr;
	m_RefCount = nullptr;
}


// comparisons
///////////////

//---------------------------------
// RefPtr::operator==
//
// Compare pointer equals null
//
template <typename TDataType>
bool operator==(RefPtr<TDataType> const& ptr, std::nullptr_t)
{
	return ptr.IsNull();
}

template <typename TDataType>
bool operator==(std::nullptr_t, RefPtr<TDataType> const& ptr)
{
	return ptr.IsNull();
}

template <typename TDataType>
bool operator==(RefPtr<TDataType> const& ptr1, RefPtr<TDataType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType>
bool operator!=(RefPtr<TDataType> const& ptr, std::nullptr_t)
{
	return !ptr.IsNull();
}

template <typename TDataType>
bool operator!=(std::nullptr_t, RefPtr<TDataType> const& ptr)
{
	return !ptr.IsNull();
}

template <typename TDataType>
bool operator!=(RefPtr<TDataType> const& ptr1, RefPtr<TDataType> const& ptr2)
{
	return !(ptr1 == ptr2);
}


} // namespace et
