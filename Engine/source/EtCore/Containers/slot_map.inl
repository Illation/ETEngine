#pragma once


namespace core {


//==========
// Slot Map
//==========


// construct destruct
//////////////////////

//------------------
// slot_map::c-tor
//
// move constructor
//
template <class TType, class TIndexType>
slot_map<TType, TIndexType>::slot_map(slot_map&& moving)
{
	moving.swap(*this);
}

//------------------
// slot_map::operator=
//
// move assignment
//
template <class TType, class TIndexType>
slot_map& slot_map<TType, TIndexType>::operator=(slot_map&& moving)
{
	clear();
	moving.swap(*this);
	return *this;
}


// Accessors
/////////////

//----------------------
// slot_map::operator[]
//
// access to data by ID
//
template <class TType, class TIndexType>
slot_map::reference slot_map<TType, TIndexType>::operator[](id_type const id)
{
	ET_ASSERT(m_Indices.size() > id);
	ET_ASSERT(m_Data.size() > m_Indices[id]);

	return m_Data[m_Indices[id]];
}

//----------------------
// slot_map::operator[]
//
template <class TType, class TIndexType>
slot_map::const_reference slot_map<TType, TIndexType>::operator[](id_type const id) const
{
	ET_ASSERT(is_valid(id));

	return m_Data[m_Indices[id]];
}

//----------------------
// slot_map::get
//
template <class TType, class TIndexType>
slot_map::pointer slot_map<TType, TIndexType>::at(id_type const id)
{
	return is_valid(id) ? &(*this)[id] : nullptr;
}

//----------------------
// slot_map::get
//
template <class TType, class TIndexType>
slot_map::const_pointer slot_map<TType, TIndexType>::at(id_type const id) const
{
	return is_valid(id) ? &(*this)[id] : nullptr;
}

//----------------------
// slot_map::data
//
template <class TType, class TIndexType>
slot_map::pointer slot_map<TType, TIndexType>::data()
{
	return m_Data.data();
}

//----------------------
// slot_map::data
//
template <class TType, class TIndexType>
slot_map::const_pointer slot_map<TType, TIndexType>::data() const
{
	return m_Data.data();
}

//-----------------------
// slot_map::iterator_id
//
template <class TType, class TIndexType> 
slot_map::id_type slot_map<TType, TIndexType>::iterator_id(iterator const it) const
{
	return m_IndexPositions[it - m_Data.cbegin()];
}

//-----------------------
// slot_map::iterator_id
//
template <class TType, class TIndexType>
slot_map::id_type slot_map<TType, TIndexType>::iterator_id(const_iterator const it) const
{
	return m_IndexPositions[it - m_Data.cbegin()];
}

//--------------------
// slot_map::is_valid
//
template <class TType, class TIndexType>
bool slot_map<TType, TIndexType>::is_valid(id_type const id) const
{
	return (id < m_Indices.size()) && (m_Indices[id] < m_Data.size());
}


// iterators
/////////////

//--------------------
// slot_map::begin
//
template <class TType, class TIndexType >
slot_map::iterator slot_map<TType, TIndexType>::begin()
{
	return m_Data.begin();
}

//--------------------
// slot_map::begin
//
template <class TType, class TIndexType >
slot_map::const_iterator slot_map<TType, TIndexType>::begin() const
{
	return m_Data.cbegin();
}

//--------------------
// slot_map::cbegin
//
template <class TType, class TIndexType >
slot_map::const_iterator slot_map<TType, TIndexType>::cbegin() const
{
	return m_Data.cbegin();
}

//--------------------
// slot_map::end
//
template <class TType, class TIndexType >
slot_map::iterator slot_map<TType, TIndexType>::end()
{
	return m_Data.end();
}

//--------------------
// slot_map::end
//
template <class TType, class TIndexType >
slot_map::const_iterator slot_map<TType, TIndexType>::end() const
{
	return m_Data.cend();
}

//--------------------
// slot_map::cend
//
template <class TType, class TIndexType >
slot_map::const_iterator slot_map<TType, TIndexType>::cend() const
{
	return m_Data.cend();
}

//--------------------
// slot_map::rbegin
//
template <class TType, class TIndexType >
slot_map::reverse_iterator slot_map<TType, TIndexType>::rbegin()
{
	return m_Data.rbegin();
}

//--------------------
// slot_map::rbegin
//
template <class TType, class TIndexType >
slot_map::const_reverse_iterator slot_map<TType, TIndexType>::rbegin() const
{
	return m_Data.crbegin();
}

//--------------------
// slot_map::crbegin
//
template <class TType, class TIndexType >
slot_map::const_reverse_iterator slot_map<TType, TIndexType>::crbegin() const
{
	return m_Data.crbegin();
}

//--------------------
// slot_map::end
//
template <class TType, class TIndexType >
slot_map::reverse_iterator slot_map<TType, TIndexType>::rend()
{
	return m_Data.rend();
}

//--------------------
// slot_map::rend
//
template <class TType, class TIndexType >
slot_map::const_reverse_iterator slot_map<TType, TIndexType>::rend() const
{
	return m_Data.crend();
}

//--------------------
// slot_map::crend
//
template <class TType, class TIndexType >
slot_map::const_reverse_iterator slot_map<TType, TIndexType>::crend() const
{
	return m_Data.crend();
}


// capacity
/////////////

//--------------------
// slot_map::empty
//
template <class TType, class TIndexType>
bool slot_map<TType, TIndexType>::empty() const
{
	return m_Data.empty();
}

//--------------------
// slot_map::size
//
template <class TType, class TIndexType>
slot_map::size_type slot_map<TType, TIndexType>::size() const
{
	return m_Data.size();
}

//--------------------
// slot_map::max_size
//
template <class TType, class TIndexType>
slot_map::size_type slot_map<TType, TIndexType>::max_size() const
{
	return s_InvalidIndex - 1;
}

//--------------------
// slot_map::size
//
template <class TType, class TIndexType>
slot_map::size_type slot_map<TType, TIndexType>::capacity() const
{
	return static_cast<size_type>(std::max(m_Data.capacity(), static_cast<std::vector<TType>::size_type>(max_size())));
}


// functionality
//////////////////


//--------------------
// slot_map::erase
//
template <class TType, class TIndexType>
slot_map::iterator slot_map<TType, TIndexType>::erase(iterator const it)
{
	iterator const next = std::next(it);
	erase(iterator_id(it));
	return next;
}

//--------------------
// slot_map::erase
//
template <class TType, class TIndexType>
slot_map::const_iterator slot_map<TType, TIndexType>::erase(const_iterator const it)
{
	const_iterator const next = std::next(it);
	erase(iterator_id(it));
	return next;
}

//--------------------
// slot_map::clear
//
template <class TType, class TIndexType>
void slot_map<TType, TIndexType>::clear()
{
	m_Data.clear();
	m_Indices.clear();
	m_IndexPositions.clear();
	m_FreeHead = s_InvalidIndex;
}

//--------------------
// slot_map::swap
//
template <class TType, class TIndexType>
void slot_map<TType, TIndexType>::swap(slot_map& other)
{
	std::swap(m_Data, other.m_Data);
	std::swap(m_Indices, other.m_Indices);
	std::swap(m_IndexPositions, other.m_IndexPositions);
	std::swap(m_FreeHead, other.m_FreeHead);
}

//--------------------
// slot_map::reserve
//
template <class TType, class TIndexType>
void slot_map<TType, TIndexType>::reserve(size_type const new_cap)
{
	m_Data.reserve(static_cast<std::vector<TType>::size_type>(new_cap));
	m_Indices.reserve(static_cast<std::vector<index_type>::size_type>(new_cap));
	m_IndexPositions.reserve(static_cast<std::vector<index_type>::size_type>(new_cap));
}


} // namespace core

