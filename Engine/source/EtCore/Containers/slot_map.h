#pragma once
#include <vector>


namespace core {


// we should rarely have to store more than 2 ^ 32 elements
typedef uint32 T_DefaultSlotMapIndexType;


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
template <class TType, class TIndexType = T_DefaultSlotMapIndexType>
class slot_map final
{
	// definitions
	//-------------
public:
	using index_type = TIndexType;
	using size_type = index_type; 
	using id_type = index_type;		

	using value_type = TType;
	using reference = TType&;
	using const_reference = TType const&;
	using pointer = TType*;
	using const_pointer = TType const*;

	using iterator = std::vector<TType>::iterator;
	using const_iterator = std::vector<TType>::const_iterator;
	using reverse_iterator = std::vector<TType>::reverse_iterator;
	using const_reverse_iterator = std::vector<TType>::const_reverse_iterator;
	using differnce_type = std::vector<TType>::difference_type;

private:
	static constexpr index_type s_InvalidIndex = std::numeric_limits<index_type>::max();

	// construct destruct
	//--------------------
public:
	slot_map() = default;
	slot_map(slot_map const& copy) = default;
	slot_map(slot_map&& moving);

	slot_map& operator=(slot_map const& rhs);
	slot_map& operator=(slot_map&& moving);

	// accessors
	//-----------
	reference operator[](id_type const id);
	const_reference operator[](id_type const id) const;

	pointer get(id_type const id);
	const_pointer get(id_type const id) const;

	pointer data();
	const_pointer data() const;

	id_type iterator_id(iterator const it) const;
	id_type iterator_id(const_iterator const it) const;

	// iterators
	//-----------
	iterator begin();
	iterator end();

	const_iterator begin() const;
	const_iterator end() const;

	const_iterator cbegin() const;
	const_iterator cend() const;

	reverse_iterator rbegin();
	reverse_iterator rend();

	const_reverse_iterator rbegin() const;
	const_reverse_iterator rend() const;

	const_reverse_iterator crbegin() const;
	const_reverse_iterator crend() const;

	// capacity
	//----------
	bool empty() const;
	size_type size() const;
	size_type max_size() const;
	size_type capacity() const;

	// functionality
	//---------------
	std::pair<iterator, id_type> insert(TType const& value);
	std::pair<iterator, id_type> insert(TType&& moving_value);
	
	void erase(id_type const id);
	iterator erase(iterator const it);
	const_iterator erase(const_iterator const it);
	void clear();

	void swap(slot_map& other);

	void reserve(size_type const new_cap);

	// Data
	///////

private:
	std::vector<TType> m_Data;
	std::vector<index_type> m_Indices;
	std::vector<index_type> m_IndexPositions;

	index_type m_FreeHead = s_InvalidIndex;
};


} // namespace core