#pragma once


namespace et {


//=========
// Pointer
//=========


// static functionality
////////////////////////

//--------------------
// Ptr::CreateFromRaw
//
template <typename TDataType>
Ptr<TDataType> Ptr<TDataType>::CreateFromRaw(TDataType* const raw)
{
	Ptr ret;
	ret.m_Ptr = raw;
	return ret;
}

//-----------------
// Ptr::StaticCast
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType> Ptr<TDataType>::StaticCast(Ptr<TOtherType> const& from)
{
	Ptr ret;
	ret.m_Ptr = static_cast<TDataType*>(from.m_Ptr);
	return ret;
}


// From nullptr
////////////////

//------------
// Ptr::c-tor
//
template <typename TDataType>
Ptr<TDataType>& Ptr<TDataType>::operator=(std::nullptr_t)
{
	m_Ptr = nullptr;
	return *this;
}


// Copy
////////

//---------------------------------
// Ptr::c-tor
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>::Ptr(Ptr<TOtherType> const& copy)
	: m_Ptr(copy.m_Ptr)
{ }

//---------------------------------
// Ptr::operator=
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>& Ptr<TDataType>::operator=(Ptr<TOtherType> const& copy)
{
	m_Ptr = copy.m_Ptr;
	return *this;
}


// From Unique
///////////////

//---------------------------------
// Ptr::c-tor
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>::Ptr(UniquePtr<TOtherType> const& unique)
	: m_Ptr(unique.m_Ptr)
{ }

//---------------------------------
// Ptr::operator=
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>& Ptr<TDataType>::operator=(UniquePtr<TOtherType> const& unique)
{
	m_Ptr = unique.m_Ptr;
	return *this;
}


// From Ref
////////////

//---------------------------------
// Ptr::c-tor
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>::Ptr(RefPtr<TOtherType> const& ref)
	: m_Ptr(ref.m_Ptr)
{ }

//---------------------------------
// Ptr::operator=
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>& Ptr<TDataType>::operator=(RefPtr<TOtherType> const& ref)
{
	m_Ptr = ref.m_Ptr;
	return *this;
}


// From Weak
/////////////

//---------------------------------
// Ptr::c-tor
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>::Ptr(WeakPtr<TOtherType> const& weak)
	: m_Ptr(weak.m_Ptr)
{ }

//---------------------------------
// Ptr::operator=
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>& Ptr<TDataType>::operator=(WeakPtr<TOtherType> const& weak)
{
	m_Ptr = weak.m_Ptr;
	return *this;
}


// move from refptr
////////////////////

//---------------------------------
// Ptr::c-tor
//
// Move constructor - invalidate old pointer
//
template <typename TDataType>
Ptr<TDataType>::Ptr(Ptr<TDataType>&& moving)
{
	Swap(moving);
}

//---------------------------------
// Ptr::operator=
//
// Move assignment - invalidate old pointer
//
template <typename TDataType>
Ptr<TDataType>& Ptr<TDataType>::operator=(Ptr<TDataType>&& moving)
{
	m_Ptr = nullptr;
	Swap(moving);
	return *this;
}

//---------------------------------
// Ptr::c-tor
//
// Move constructor - invalidate old pointer
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>::Ptr(Ptr<TOtherType>&& moving)
{
	m_Ptr = moving.m_Ptr;
	moving.m_Ptr = nullptr;
}

//---------------------------------
// Ptr::operator=
//
// Move assignment - invalidate old pointer
//
template <typename TDataType>
template <typename TOtherType>
Ptr<TDataType>& Ptr<TDataType>::operator=(Ptr<TOtherType>&& moving)
{
	m_Ptr = moving.m_Ptr;
	moving.m_Ptr = nullptr;
	return *this;
}


// Utility
/////////////

//---------------------------------
// Ptr::IsNull
//
template <typename TDataType>
inline bool Ptr<TDataType>::IsNull() const
{
	return (m_Ptr == nullptr);
}

//---------------------------------
// Ptr::Swap
//
template <typename TDataType>
inline void Ptr<TDataType>::Swap(Ptr<TDataType>& other) noexcept
{
	std::swap(m_Ptr, other.m_Ptr);
}


// Accessors
/////////////

//---------------------------------
// Ptr::Get
//
template <typename TDataType>
inline TDataType* Ptr<TDataType>::Get() const
{
	return m_Ptr;
}

//---------------------------------
// Ptr::operator->
//
template <typename TDataType>
inline TDataType* Ptr<TDataType>::operator->() const
{
	ET_ASSERT_PARANOID(!IsNull());
	return Get();
}

//---------------------------------
// Ptr::operator*
//
template <typename TDataType>
inline TDataType& Ptr<TDataType>::operator*() const
{
	ET_ASSERT_PARANOID(!IsNull());
	return *Get();
}


// comparisons
///////////////

template <typename TDataType>
bool operator==(Ptr<TDataType> const& ptr, std::nullptr_t)
{
	return ptr.IsNull();
}

template <typename TDataType>
bool operator==(std::nullptr_t, Ptr<TDataType> const& ptr)
{
	return ptr.IsNull();
}

template <typename TDataType, typename TOtherType>
bool operator==(Ptr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType, typename TOtherType>
bool operator==(Ptr<TDataType> const& ptr1, UniquePtr<TOtherType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType, typename TOtherType>
bool operator==(UniquePtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType, typename TOtherType>
bool operator==(Ptr<TDataType> const& ptr1, RefPtr<TOtherType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType, typename TOtherType>
bool operator==(RefPtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType, typename TOtherType>
bool operator==(Ptr<TDataType> const& ptr1, WeakPtr<TOtherType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType, typename TOtherType>
bool operator==(WeakPtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2)
{
	return (ptr1.Get() == ptr2.Get());
}

template <typename TDataType>
bool operator!=(Ptr<TDataType> const& ptr, std::nullptr_t)
{
	return !ptr.IsNull();
}

template <typename TDataType>
bool operator!=(std::nullptr_t, Ptr<TDataType> const& ptr)
{
	return !ptr.IsNull();
}

template <typename TDataType, typename TOtherType>
bool operator!=(Ptr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType, typename TOtherType>
bool operator!=(Ptr<TDataType> const& ptr1, UniquePtr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType, typename TOtherType>
bool operator!=(UniquePtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType, typename TOtherType>
bool operator!=(Ptr<TDataType> const& ptr1, RefPtr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType, typename TOtherType>
bool operator!=(RefPtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType, typename TOtherType>
bool operator!=(Ptr<TDataType> const& ptr1, WeakPtr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}

template <typename TDataType, typename TOtherType>
bool operator!=(WeakPtr<TDataType> const& ptr1, Ptr<TOtherType> const& ptr2)
{
	return !(ptr1 == ptr2);
}


} // namespace et


// REFLECTION
//////////////////////////////////////////////////////////////////////////////////////


namespace rttr {


template<typename TDataType>
struct wrapper_mapper<et::Ptr<TDataType>>
{
	using wrapped_type = TDataType * ;
	using type = et::Ptr<TDataType>;

	static RTTR_INLINE wrapped_type get(type const& obj)
	{
		return obj.Get();
	}

	// can't create	
};


} // namespace rttr
