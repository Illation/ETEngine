#pragma once


namespace et {


//==============
// Weak Pointer
//==============


// From nullptr
////////////////

//------------------
// WeakPtr::c-tor
//
template <typename TDataType>
WeakPtr<TDataType>& WeakPtr<TDataType>::operator=(std::nullptr_t)
{
	Invalidate();
	return *this;
}


// copy from other reference
/////////////////////////////

//---------------------------------
// WeakPtr::c-tor
//
// copy and increment the weak refcount
//
template <typename TDataType>
WeakPtr<TDataType>::WeakPtr(WeakPtr<TDataType> const& copy)
	: m_Ptr(copy.m_Ptr)
	, m_RefCount(copy.m_RefCount)
{
	if (m_RefCount != nullptr)
	{
		++(m_RefCount->m_WeakReferences);
	}
}

//---------------------------------
// WeakPtr::operator=
//
// Copy assignment - increment weak refcount - using copy and swap idiom
//
template <typename TDataType>
WeakPtr<TDataType>& WeakPtr<TDataType>::operator=(WeakPtr<TDataType> copy)
{
	copy.Swap(*this);
	return *this;
}

//---------------------------------
// WeakPtr::c-tor
//
// copy and increment the weak refcount
// Version supporting derived classes
//
template <typename TDataType>
template <typename TOtherType>
WeakPtr<TDataType>::WeakPtr(WeakPtr<TOtherType> const& copy)
	: m_RefCount(copy.m_RefCount)
{
	TDataType* tempPtr = copy.m_Ptr;
	std::swap(m_Ptr, tempPtr);
	if (m_RefCount != nullptr)
	{
		++(m_RefCount->m_WeakReferences);
	}
}

//---------------------------------
// WeakPtr::operator=
//
// Copy assignment - increment weak refcount
// Version supporting derived classes
//
template <typename TDataType>
template <typename TOtherType>
WeakPtr<TDataType>& WeakPtr<TDataType>::operator=(WeakPtr<TOtherType> const& copy)
{
	TDataType* tempPtr = copy.m_Ptr;
	std::swap(m_Ptr, tempPtr);

	m_RefCount = copy.m_RefCount;
	if (m_RefCount != nullptr)
	{
		++(m_RefCount->m_WeakReferences);
	}

	return *this;
}


// move from weak
///////////////////

//---------------------------------
// WeakPtr::c-tor
//
// Move constructor - invalidate old pointer
//
template <typename TDataType>
template <typename TOtherType>
WeakPtr<TDataType>::WeakPtr(WeakPtr<TOtherType>&& moving)
{
	// copy data. moved refcount stays intact, our own refcount will be 0 anyway
	TDataType* tempPtr = moving.m_Ptr;
	std::swap(m_Ptr, tempPtr);
	std::swap(m_RefCount, moving.m_RefCount);

	// invalidate old pointer
	moving.m_Ptr = nullptr;
}

//---------------------------------
// WeakPtr::operator=
//
// Move assignment - invalidate old pointer
//
template <typename TDataType>
template <typename TOtherType>
WeakPtr<TDataType>& WeakPtr<TDataType>::operator=(WeakPtr<TOtherType>&& moving)
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
// WeakPtr::c-tor
//
// Create from ref pointer
//
template <typename TDataType>
template <typename TOtherType>
WeakPtr<TDataType>::WeakPtr(RefPtr<TOtherType> const& ref)
{
	// copy data. moved refcount stays intact, our own refcount will be 0 anyway
	TDataType* tempPtr = ref.Get();
	std::swap(m_Ptr, tempPtr);
	m_RefCount = ref.m_RefCount;

	StartRefCount();
}

//---------------------------------
// WeakPtr::operator=
//
// Create from ref pointer
//
template <typename TDataType>
template <typename TOtherType>
WeakPtr<TDataType>& WeakPtr<TDataType>::operator=(RefPtr<TOtherType> const& ref)
{
	// if we hold a reference remove it
	DecrementRefcount();

	// copy data. moved refcount stays intact
	TDataType* tempPtr = ref.Get();
	std::swap(m_Ptr, tempPtr);
	m_RefCount = ref.m_RefCount;

	StartRefCount();

	return *this;
}


// destroy
/////////////

//---------------------------------
// WeakPtr::d-tor
//
// decrement ref count, delete data if there are no reference, delete refcount if there aren't even any weak references
//
template <typename TDataType>
WeakPtr<TDataType>::~WeakPtr()
{
	DecrementRefcount();
}


// Utility
/////////////

//---------------------------------
// WeakPtr::IsNull
//
template <typename TDataType>
bool WeakPtr<TDataType>::IsNull() const
{
	return ((m_Ptr == nullptr) || (m_RefCount == nullptr) || (m_RefCount->m_References == 0u));
}

//---------------------------------
// WeakPtr::Swap
//
// Swap two unique pointers
//
template <typename TDataType>
void WeakPtr<TDataType>::Swap(WeakPtr<TDataType>& other) noexcept
{
	std::swap(m_Ptr, other.m_Ptr);
	std::swap(m_RefCount, other.m_RefCount);
}


// Accessors
/////////////

//---------------------------------
// WeakPtr::Get
//
template <typename TDataType>
TDataType* WeakPtr<TDataType>::Get() const
{
	return m_Ptr;
}

//---------------------------------
// WeakPtr::operator->
//
template <typename TDataType>
TDataType* WeakPtr<TDataType>::operator->() const
{
	ET_ASSERT(!IsNull());
	return Get();
}

//---------------------------------
// WeakPtr::operator*
//
template <typename TDataType>
TDataType& WeakPtr<TDataType>::operator*() const
{
	ET_ASSERT(!IsNull());
	return *Get();
}


// private
///////////

//---------------------------------
// WeakPtr::StartRefCount
//
template <typename TDataType>
void et::WeakPtr<TDataType>::StartRefCount()
{
	if (m_RefCount != nullptr)
	{
		m_RefCount->m_WeakReferences++;
	}
}

//---------------------------------
// WeakPtr::DecrementRefcount
//
template <typename TDataType>
void WeakPtr<TDataType>::DecrementRefcount()
{
	if (m_RefCount != nullptr)
	{
		--(m_RefCount->m_WeakReferences);
		if ((m_RefCount->m_WeakReferences == 0u) && (m_RefCount->m_References == 0u))
		{
			delete m_RefCount;
		}
	}
}

//---------------------------------
// WeakPtr::DecrementRefcount
//
template <typename TDataType>
void WeakPtr<TDataType>::Invalidate()
{
	DecrementRefcount();
	m_Ptr = nullptr;
	m_RefCount = nullptr;
}


// comparisons
///////////////

//---------------------------------
// WeakPtr::operator==
//
// Compare pointer equals null
//
template <typename TDataType>
bool operator==(WeakPtr<TDataType> const& ptr, std::nullptr_t)
{
	return ptr.IsNull();
}

template <typename TDataType>
bool operator==(std::nullptr_t, WeakPtr<TDataType> const& ptr)
{
	return ptr.IsNull();
}

template <typename TDataType>
bool operator==(WeakPtr<TDataType> const& ptr1, WeakPtr<TDataType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType>
bool operator==(WeakPtr<TDataType> const& ptr1, RefPtr<TDataType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType>
bool operator==(RefPtr<TDataType> const& ptr1, WeakPtr<TDataType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType>
bool operator!=(WeakPtr<TDataType> const& ptr, std::nullptr_t)
{
	return !ptr.IsNull();
}

template <typename TDataType>
bool operator!=(std::nullptr_t, WeakPtr<TDataType> const& ptr)
{
	return !ptr.IsNull();
}

template <typename TDataType>
bool operator!=(WeakPtr<TDataType> const& ptr1, WeakPtr<TDataType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType>
bool operator!=(WeakPtr<TDataType> const& ptr1, RefPtr<TDataType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType>
bool operator!=(RefPtr<TDataType> const& ptr1, WeakPtr<TDataType> const& ptr2)
{
	return !(ptr1 == ptr2);
}


} // namespace et
