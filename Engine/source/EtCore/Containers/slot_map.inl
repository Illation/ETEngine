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
template <class TType>
slot_map<TType>::slot_map(slot_map&& moving)
{
	moving.swap(*this);
}

//------------------
// slot_map::operator=
//
// move assignment
//
template <class TType>
slot_map<TType>& slot_map<TType>::operator=(slot_map&& moving)
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
template <class TType>
TType& slot_map<TType>::operator[](id_type const id)
{
	ET_ASSERT(is_valid(id));

	return m_Data[m_Indices[id]];
}

//----------------------
// slot_map::operator[]
//
template <class TType>
TType const& slot_map<TType>::operator[](id_type const id) const
{
	ET_ASSERT(is_valid(id));

	return m_Data[m_Indices[id]];
}

//----------------------
// slot_map::get
//
template <class TType>
TType* slot_map<TType>::at(id_type const id)
{
	return is_valid(id) ? &(*this)[id] : nullptr;
}

//----------------------
// slot_map::get
//
template <class TType>
TType const* slot_map<TType>::at(id_type const id) const
{
	return is_valid(id) ? &(*this)[id] : nullptr;
}

//----------------------
// slot_map::data
//
template <class TType>
TType* slot_map<TType>::data()
{
	return m_Data.data();
}

//----------------------
// slot_map::data
//
template <class TType>
TType const* slot_map<TType>::data() const
{
	return m_Data.data();
}

//-----------------------
// slot_map::iterator_id
//
template <class TType> 
typename slot_map<TType>::id_type slot_map<TType>::iterator_id(iterator const it) const
{
	return m_IndexPositions[it - m_Data.cbegin()];
}

//-----------------------
// slot_map::iterator_id
//
template <class TType>
typename slot_map<TType>::id_type slot_map<TType>::iterator_id(const_iterator const it) const
{
	return m_IndexPositions[it - m_Data.cbegin()];
}

//---------------
// slot_map::ids
//
template <class TType>
std::vector<typename slot_map<TType>::id_type> const& slot_map<TType>::ids() const
{
	return m_IndexPositions;
}

//-----------------------
// slot_map::get_iterator
//
template <class TType> 
typename slot_map<TType>::iterator slot_map<TType>::get_iterator(id_type const id)
{
	ET_ASSERT(m_Indices.size() > id);
	return begin() + m_Indices[id];
}

//-----------------------
// slot_map::get_iterator
//
template <class TType>
typename slot_map<TType>::const_iterator slot_map<TType>::get_iterator(id_type const id) const
{
	ET_ASSERT(m_Indices.size() > id);
	return cbegin() + m_Indices[id];
}

//--------------------
// slot_map::is_valid
//
template <class TType>
bool slot_map<TType>::is_valid(id_type const id) const
{
	return (id < m_Indices.size()) && (m_Indices[id] < m_Data.size());
}


// iterators
/////////////

//--------------------
// slot_map::begin
//
template <class TType>
typename slot_map<TType>::iterator slot_map<TType>::begin()
{
	return m_Data.begin();
}

//--------------------
// slot_map::begin
//
template <class TType>
typename slot_map<TType>::const_iterator slot_map<TType>::begin() const
{
	return m_Data.cbegin();
}

//--------------------
// slot_map::cbegin
//
template <class TType>
typename slot_map<TType>::const_iterator slot_map<TType>::cbegin() const
{
	return m_Data.cbegin();
}

//--------------------
// slot_map::end
//
template <class TType>
typename slot_map<TType>::iterator slot_map<TType>::end()
{
	return m_Data.end();
}

//--------------------
// slot_map::end
//
template <class TType>
typename slot_map<TType>::const_iterator slot_map<TType>::end() const
{
	return m_Data.cend();
}

//--------------------
// slot_map::cend
//
template <class TType>
typename slot_map<TType>::const_iterator slot_map<TType>::cend() const
{
	return m_Data.cend();
}

//--------------------
// slot_map::rbegin
//
template <class TType>
typename slot_map<TType>::reverse_iterator slot_map<TType>::rbegin()
{
	return m_Data.rbegin();
}

//--------------------
// slot_map::rbegin
//
template <class TType>
typename slot_map<TType>::const_reverse_iterator slot_map<TType>::rbegin() const
{
	return m_Data.crbegin();
}

//--------------------
// slot_map::crbegin
//
template <class TType>
typename slot_map<TType>::const_reverse_iterator slot_map<TType>::crbegin() const
{
	return m_Data.crbegin();
}

//--------------------
// slot_map::end
//
template <class TType>
typename slot_map<TType>::reverse_iterator slot_map<TType>::rend()
{
	return m_Data.rend();
}

//--------------------
// slot_map::rend
//
template <class TType>
typename slot_map<TType>::const_reverse_iterator slot_map<TType>::rend() const
{
	return m_Data.crend();
}

//--------------------
// slot_map::crend
//
template <class TType>
typename slot_map<TType>::const_reverse_iterator slot_map<TType>::crend() const
{
	return m_Data.crend();
}


// capacity
/////////////

//--------------------
// slot_map::empty
//
template <class TType>
bool slot_map<TType>::empty() const
{
	return m_Data.empty();
}

//--------------------
// slot_map::size
//
template <class TType>
typename slot_map<TType>::size_type slot_map<TType>::size() const
{
	return static_cast<size_type>(m_Data.size());
}

//--------------------
// slot_map::max_size
//
template <class TType>
typename slot_map<TType>::size_type slot_map<TType>::max_size() const
{
	return s_InvalidIndex - 1;
}

//--------------------
// slot_map::size
//
template <class TType>
typename slot_map<TType>::size_type slot_map<TType>::capacity() const
{
	return static_cast<size_type>(std::max(m_Data.capacity(), static_cast<std::vector<TType>::size_type>(max_size())));
}


// functionality
//////////////////

//--------------------
// slot_map::erase
//
template <class TType>
void slot_map<TType>::erase(id_type const id)
{
	ET_ASSERT(is_valid(id));
	if (size() <= static_cast<index_type>(1))
	{
		clear();
		return;
	}

	iterator const it = get_iterator(id);
	iterator const last = std::prev(end());
	m_Indices[iterator_id(last)] = m_Indices[id];

	std::iter_swap(it, last);
	m_Data.pop_back();
	m_IndexPositions[m_Indices[id]] = m_IndexPositions[m_IndexPositions.size() - 1];
	m_IndexPositions.pop_back();

	m_Indices[id] = m_FreeHead;
	m_FreeHead = id;
}

//--------------------
// slot_map::erase
//
template <class TType>
typename slot_map<TType>::iterator slot_map<TType>::erase(iterator const it)
{
	iterator const next = std::next(it);
	erase(iterator_id(it));
	return next;
}

//--------------------
// slot_map::erase
//
template <class TType>
typename slot_map<TType>::const_iterator slot_map<TType>::erase(const_iterator const it)
{
	const_iterator const next = std::next(it);
	erase(iterator_id(it));
	return next;
}

//--------------------
// slot_map::clear
//
template <class TType>
void slot_map<TType>::clear()
{
	m_Data.clear();
	m_Indices.clear();
	m_IndexPositions.clear();
	m_FreeHead = s_InvalidIndex;
}

//--------------------
// slot_map::swap
//
template <class TType>
void slot_map<TType>::swap(slot_map& other)
{
	std::swap(m_Data, other.m_Data);
	std::swap(m_Indices, other.m_Indices);
	std::swap(m_IndexPositions, other.m_IndexPositions);
	std::swap(m_FreeHead, other.m_FreeHead);
}

//--------------------
// slot_map::reserve
//
template <class TType>
void slot_map<TType>::reserve(size_type const new_cap)
{
	m_Data.reserve(static_cast<std::vector<TType>::size_type>(new_cap));
	m_Indices.reserve(static_cast<std::vector<index_type>::size_type>(new_cap));
	m_IndexPositions.reserve(static_cast<std::vector<index_type>::size_type>(new_cap));
}


// utility
///////////

//------------------------
// slot_map::insert_impl
//
//template <class TType>
//std::pair<slot_map::iterator, typename slot_map<TType>::id_type> slot_map<TType>::insert_impl(TType&& value)
//{
//	index_type const dataPos = static_cast<index_type>(m_Data.size());
//
//	index_type indexPos;
//	if (m_FreeHead == s_InvalidIndex)
//	{
//		indexPos = static_cast<index_type>(m_Indices.size());
//		m_Indices.push_back(dataPos);
//	}
//	else
//	{
//		indexPos = m_FreeHead;
//		m_FreeHead = m_Indices[m_FreeHead];
//		m_Indices[indexPos] = dataPos;
//	}
//
//	m_Data.push_back(std::forward(<TType>(value)));
//	m_IndexPositions.push_back(indexPos);
//
//	return std::make_pair(get_iterator(indexPos), indexPos);
//}


} // namespace core

