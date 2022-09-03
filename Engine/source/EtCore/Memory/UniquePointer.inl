#pragma once


namespace et {


//================
// Unique Pointer
//================


// static functionality
////////////////////////

//-----------------------
// UniquePtr::StaticCast
//
template <typename TDataType>
template <typename TOtherType>
UniquePtr<TDataType> UniquePtr<TDataType>::StaticCast(UniquePtr<TOtherType>&& from)
{
	UniquePtr ret;
	ret.m_Ptr = static_cast<TDataType*>(from.Release());
	return ret;
}


// From nullptr
////////////////

//------------------
// UniquePtr::c-tor
//
template <typename TDataType>
UniquePtr<TDataType>& UniquePtr<TDataType>::operator=(std::nullptr_t)
{
	Reset();
	return *this;
}


// From creator
////////////////

//------------------
// UniquePtr::c-tor
//
// Move data in from creation and invalidate creation class
//
template <typename TDataType>
template <typename TOtherType>
UniquePtr<TDataType>::UniquePtr(Create<TOtherType>&& create)
{
	UniquePtr<TDataType> tmp;
	tmp.m_Ptr = create.Release();
	tmp.Swap(*this);
}

//---------------------------------
// UniquePtr::operator=
//
// Move data in from creation and invalidate creation class
//
template <typename TDataType>
template <typename TOtherType>
UniquePtr<TDataType>& UniquePtr<TDataType>::operator=(Create<TOtherType>&& create)
{
	UniquePtr<TDataType> tmp; 
	tmp.m_Ptr = create.Release();
	tmp.Swap(*this); // since we swap our current pointer, when temp goes out of scope the memory for what was our data is freed
	return *this;
}


// Move another UniquePtr
/////////////////////////

//---------------------------------
// UniquePtr::c-tor
//
// Move constructor - invalidate old pointer
//
template <typename TDataType>
UniquePtr<TDataType>::UniquePtr(UniquePtr<TDataType>&& moving)
	: m_Ptr(moving.Release())
{}

//---------------------------------
// UniquePtr::operator=
//
// Move assignment - invalidate old pointer
//
template <typename TDataType>
UniquePtr<TDataType>& UniquePtr<TDataType>::operator=(UniquePtr<TDataType>&& moving)
{
	m_Ptr = moving.Release();
	return *this;
}

//---------------------------------
// UniquePtr::c-tor
//
// Move constructor - invalidate old pointer
// version for derived classes
//
template <typename TDataType>
template <typename TOtherType>
UniquePtr<TDataType>::UniquePtr(UniquePtr<TOtherType>&& moving)
{
	// We can not use swap directly.
	// Even though TOtherType is derived from TDataType, the reverse is not true.
	// So we have put it in a temporary locally first.
	UniquePtr<TDataType> tmp;
	tmp.m_Ptr = moving.Release();
	tmp.Swap(*this);
}

//---------------------------------
// UniquePtr::operator=
//
// Move assignment - invalidate old pointer
// version for derived classes
//
template <typename TDataType>
template <typename TOtherType>
UniquePtr<TDataType>& UniquePtr<TDataType>::operator=(UniquePtr<TOtherType>&& moving)
{
	UniquePtr<TDataType> tmp;
	tmp.m_Ptr = moving.Release();
	tmp.Swap(*this);
	return *this;
}


// destroy
/////////////

//---------------------------------
// UniquePtr::d-tor
//
// delete internal pointer if it's not null
//
template <typename TDataType>
UniquePtr<TDataType>::~UniquePtr()
{
	delete m_Ptr;
}


// Utility
/////////////

//---------------------------------
// UniquePtr::IsNull
//
template <typename TDataType>
bool UniquePtr<TDataType>::IsNull() const
{
	return (m_Ptr == nullptr);
}

//---------------------------------
// UniquePtr::Swap
//
// Swap two unique pointers
//
template <typename TDataType>
void UniquePtr<TDataType>::Swap(UniquePtr<TDataType>& other) noexcept
{
	std::swap(m_Ptr, other.m_Ptr);
}

//---------------------------------
// UniquePtr::Swap
//
// release data to caller
//
template <typename TDataType>
TDataType* UniquePtr<TDataType>::Release()
{
	TDataType* tmp = nullptr;
	std::swap(tmp, m_Ptr);
	return tmp;
}

//---------------------------------
// UniquePtr::Reset
//
// reset to null - free data
//
template <typename TDataType>
void UniquePtr<TDataType>::Reset()
{
	TDataType* tmp = Release();
	delete tmp;
}


// Accessors
/////////////

//---------------------------------
// UniquePtr::Get
//
template <typename TDataType>
TDataType* UniquePtr<TDataType>::Get() const
{
	return m_Ptr;
}

//---------------------------------
// UniquePtr::operator->
//
template <typename TDataType>
TDataType* UniquePtr<TDataType>::operator->() const
{
	ET_ASSERT(m_Ptr != nullptr);
	return Get();
}

//---------------------------------
// UniquePtr::operator*
//
template <typename TDataType>
TDataType& UniquePtr<TDataType>::operator*() const
{
	ET_ASSERT(m_Ptr != nullptr);
	return *Get();
}


//---------------------------------
// UniquePtr::operator==
//
// Compare pointer equals null
//
template <typename TDataType>
bool operator==(UniquePtr<TDataType> const& ptr, std::nullptr_t)
{
	return ptr.IsNull();
}

template <typename TDataType>
bool operator==(std::nullptr_t, UniquePtr<TDataType> const& ptr)
{
	return ptr.IsNull();
}

template <typename TDataType, typename TOtherType>
bool operator==(UniquePtr<TDataType> const& ptr1, UniquePtr<TOtherType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType, typename TOtherType>
bool operator == (TDataType* const ptr1, UniquePtr<TOtherType> const& ptr2)
{
	return (ptr1 == ptr2.Get());
}

template <typename TDataType, typename TOtherType>
bool operator == (UniquePtr<TDataType> const& ptr1, TOtherType const* const ptr2)
{
	return (ptr1.Get() == ptr2);
}

template <typename TDataType>
bool operator!=(UniquePtr<TDataType> const& ptr, std::nullptr_t)
{
	return !ptr.IsNull();
}

template <typename TDataType>
bool operator!=(std::nullptr_t, UniquePtr<TDataType> const& ptr)
{
	return !ptr.IsNull();
}

template <typename TDataType, typename TOtherType>
bool operator!=(UniquePtr<TDataType> const& ptr1, UniquePtr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType, typename TOtherType>
bool operator != (TDataType* const ptr1, UniquePtr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType, typename TOtherType>
bool operator != (UniquePtr<TDataType> const& ptr1, TOtherType const* const ptr2)
{
	return !(ptr1 == ptr2);
}


} // namespace et


// REFLECTION
//////////////////////////////////////////////////////////////////////////////////////


namespace rttr {


template<typename TDataType>
struct wrapper_mapper<et::UniquePtr<TDataType>>
{
	using wrapped_type = TDataType * ;
	using type = et::UniquePtr<TDataType>;

	static RTTR_INLINE wrapped_type get(type const& obj)
	{
		return obj.Get();
	}

	static RTTR_INLINE type create(wrapped_type const& t)
	{
		return type(et::Create<TDataType>(t));
	}
};


} // namespace rttr
