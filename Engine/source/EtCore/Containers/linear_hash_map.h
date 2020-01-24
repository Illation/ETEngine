#pragma once


namespace et {
namespace core {


//---------------------------------
// lin_hash_map
//
// #todo: implementation is incomplete
//
// similar to std::unordered_map, for higher performance and easy lookup in external debug visualizers
//
// This is done by using open addressing and linear probing instead of chaining 
//
// Heavily based on https://github.com/rigtorp/HashMap by Erik Rigtorp
//  - license: https://github.com/rigtorp/HashMap/blob/master/LICENSE
//
// Benefits: 
//	* Rare allocations
//  * cache efficient storage
//  * fast deletion by swapping elements
//
// Tradeoffs:
//  * Higher memory use 
//  * Memory is not reclaimed
//
template <typename TKeyType, typename TValType, typename THashFn = std::hash<TKeyType>, typename TKeyEqu = std::equal_to<void>>
class lin_hash_map final
{
	// definitions
	//-------------
public:
	using key_type = TKeyType;
	using mapped_type = TValType;
	using value_type = std::pair<TKeyType, TValType>;
	using size_type = std::size_t;
	using hasher = THashFn;
	using key_equal = TKeyEqu;
	using reference = value_type&;
	using const_reference = const value_type&;

	using T_Buckets = std::vector<value_type>;

	//---------------------------------
	// lin_iterator
	//
	// Forward iteration for the hash map
	//
	template <typename TStorageType, typename TIterVal>
	struct lin_iterator final
	{
		// definitions
		//-------------
		using difference_type = std::ptrdiff_t;
		using value_type = TIterVal;
		using pointer = value_type*;
		using reference = value_type&;
		using iterator_category = std::forward_iterator_tag;

	private:
		friend TStorageType;

		// construct destruct
		//--------------------
		explicit lin_iterator(TStorageType* const map);
		explicit lin_iterator(TStorageType* const map, size_type const idx);

		// functionality
		//---------------
	public:
		lin_iterator& operator ++();

		// accessors
		//-----------
		bool operator ==(lin_iterator const& other) const;
		bool operator !=(lin_iterator const& other) const;

		reference operator*() const;
		pointer operator->() const;

		// Data
		///////

	private:
		TStorageType* m_Container;
		typename TStorageType::size_type m_Index = 0;
	};

	using iterator = lin_iterator<lin_hash_map, value_type>;
	using const_iterator = lin_iterator<lin_hash_map const, value_type const>;

	// construct destruct
	//--------------------
	lin_hash_map(size_type const bucketCount, key_type const emptyKeyVal);
	lin_hash_map(lin_hash_map const& other, size_type const bucketCount);

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

	// accessors
	//-----------

	// access a value by key
	mapped_type& at(key_type const& key);
	mapped_type const& at(key_type const& key) const;

	// access a value by key
	mapped_type& operator[](const key_type& key);

	// number of elements of key
	size_type count(key_type const& key) const;

	// find a value by key 
	iterator find(key_type const& key);
	const_iterator find(key_type const& key) const;

	// capacity
	//----------

	// check if any data is stored
	bool empty() const;

	// count of data elements (not indices)
	size_type size() const;

	// maximum amount of elements this structure can contain
	size_type max_size() const;

	// buckets
	//---------

	// number of buckets used for storage
	size_type bucket_count() const noexcept;

	// maximum number of buckets that can be used for storage
	size_type max_bucket_count() const noexcept;

	// functionality accessors
	//--------------------------
	hasher hash_function() const { return hasher(); }
	key_equal key_eq() const { return key_equal(); }

	// functionality
	//---------------

	// add a key/value pair to the container - if there already is an element with the same key, don't insert but return an iterator of the element
	std::pair<iterator, bool> insert(value_type const& value);
	std::pair<iterator, bool> insert(value_type&& moving_value);

	// constructs the key value pair in-place and adds it
	template <typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args);

	// remove an element at the iterator position
	void erase(iterator it);

	// remove an element of the key
	size_type erase(key_type const& key);

	// remove all elements 
	void clear();

	// swap with the contents of another linear hash map
	void swap(lin_hash_map& other);

	// hashing
	//---------

	// changes the amount of buckets that are used for storage
	void rehash(size_type const bucketCount);

	// ensure a certain number of elements will fit without reallocating later
	void reserve(size_type const numElements);


	// Data
	///////

private:
	key_type m_EmptyKeyVal;
	T_Buckets m_Buckets;
	size_t m_Size;
};


} // namespace core
} // namespace et


#include "linear_hash_map.inl"
