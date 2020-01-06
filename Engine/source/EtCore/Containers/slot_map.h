#pragma once
#include <vector>


namespace core {


// we should rarely have to store more than 2 ^ 32 elements
typedef uint32 T_DefaultSlotMapIndexType;
typedef T_DefaultSlotMapIndexType T_SlotId;
static constexpr T_SlotId INVALID_SLOT_ID = std::numeric_limits<T_SlotId>::max();


//---------------------------------
// slot_map
//
// STL-like associative container with dense data storage to allow fast unordered element iteration
//
// This is done by maintaining a sparse index list for access with keys, in combination with a free and delete list
// This implementation is done without versioning
//
// Benefits: 
//	* O(1) insert, remove and access
//  * cache friendly unordered element iteration
//  * rare memory allocation during insertion similar to std::vector
//
// Tradeoffs:
//  * Higher memory use due to index list compared to std::vector
//  * Pointers to elements are unstable, keys need to be used
//  * Look up with keys use one layer of indirection -> sorted iteration is not cache friendly
//
// Memory footprint:
//	* size = (type_size + index_size + index_size * most_reclaimed_element_fraction) * num_elements
//  * if the default index type is used (32 bit ints) the footprint in 64 bit build shouldn't be significantly larger 
//     - than storing the data array + an array of pointers to each element
//
template <class TType>
class slot_map final
{
	// definitions
	//-------------
public:
	using index_type = T_DefaultSlotMapIndexType;
	using size_type = index_type; 
	using id_type = index_type;		

	using value_type = TType;
	using reference = TType&;
	using const_reference = TType const&;
	using pointer = TType*;
	using const_pointer = TType const*;

	using iterator = typename std::vector<TType>::iterator;
	using const_iterator = typename std::vector<TType>::const_iterator;
	using reverse_iterator = typename std::vector<TType>::reverse_iterator;
	using const_reverse_iterator = typename std::vector<TType>::const_reverse_iterator;
	using difference_type = typename std::vector<TType>::difference_type;

//private:
	static constexpr index_type s_InvalidIndex = std::numeric_limits<index_type>::max();

	// construct destruct
	//--------------------
//public:
	slot_map() = default;
	slot_map(slot_map const& copy) = default;
	slot_map(slot_map&& moving);

	slot_map& operator=(slot_map const& rhs) = default;
	slot_map& operator=(slot_map&& moving);

	// accessors
	//-----------

	// access to data by ID
	reference operator[](id_type const id);
	const_reference operator[](id_type const id) const;

	// return a pointer to the data or nullptr for an invalid ID
	pointer at(id_type const id);
	const_pointer at(id_type const id) const;

	// pointer to the first element being stored
	pointer data();
	const_pointer data() const;

	// convert an iterator to an ID
	id_type iterator_id(iterator const it) const;
	id_type iterator_id(const_iterator const it) const;

	// all valid ids, in no particular order
	std::vector<id_type> const& ids() const;

	// convert an ID to an iterator
	iterator get_iterator(id_type const id);
	const_iterator get_iterator(id_type const id) const;

	// check if an ID is valid
	bool is_valid(id_type const id) const;

	// iterators
	//-----------

	// iterator pointing to the first element in data
	iterator begin();
	const_iterator begin() const;
	const_iterator cbegin() const;

	// "invalid" iterator after the last element in data
	iterator end();
	const_iterator end() const;
	const_iterator cend() const;

	// iterator pointing to the first element in reversed data
	reverse_iterator rbegin();
	const_reverse_iterator rbegin() const;
	const_reverse_iterator crbegin() const;

	// "invalid" iterator pointing after the last element in reversed data
	reverse_iterator rend();
	const_reverse_iterator rend() const;
	const_reverse_iterator crend() const;

	// capacity
	//----------

	// check if any data is stored
	bool empty() const; 

	// count of data elements (not indices)
	size_type size() const; 

	// maximum amount of elements this structure can contain
	size_type max_size() const; 

	// amount of data that can currently be contained without reallocating the data
	size_type capacity() const; 

	// functionality
	//---------------
	
	// add an element to the map return a pair of an iterator pointing to the element and the elements new ID
	std::pair<iterator, id_type> insert(TType const& value) { return insert_impl(value); }
	std::pair<iterator, id_type> insert(TType&& moving_value) { return insert_impl(std::move(moving_value)); }

	// remove an element from the map
	void erase(id_type const id);
	iterator erase(iterator const it); // return an iterator to the next element
	const_iterator erase(const_iterator const it);

	// remove all elements from the map
	void clear();

	// swap the contents of the map with another maps contents
	void swap(slot_map& other);

	// ensure the maps internal arrays have enough space to keep adding elements without future reallocation
	void reserve(size_type const new_cap);

	// utility
	//---------
private:
	std::pair<iterator, id_type> insert_impl(TType&& value)
	{
		index_type const dataPos = static_cast<index_type>(m_Data.size());

		index_type indexPos;
		if (m_FreeHead == s_InvalidIndex)
		{
			indexPos = static_cast<index_type>(m_Indices.size());
			m_Indices.push_back(dataPos);
		}
		else
		{
			indexPos = m_FreeHead;
			m_FreeHead = m_Indices[m_FreeHead];
			m_Indices[indexPos] = dataPos;
		}

		m_Data.push_back(std::forward<TType>(value));
		m_IndexPositions.push_back(indexPos);

		return std::make_pair(get_iterator(indexPos), indexPos);
	}

	// Data
	///////

	std::vector<TType> m_Data;
	std::vector<index_type> m_Indices;
	std::vector<index_type> m_IndexPositions;

	index_type m_FreeHead = s_InvalidIndex;
};


} // namespace core


#include "slot_map.inl"
