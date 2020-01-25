#pragma once


namespace et {
namespace core {


//---------------------------------
// lin_hash_map
//
// similar to std::unordered_map, for higher performance and easy lookup in external debug visualizers
//
// This is done by using open addressing and linear probing instead of chaining 
//
// Benefits: 
//	* Rare allocations
//  * cache efficient storage
//  * fast deletion by swapping elements
//
// Tradeoffs:
//  * Higher memory use - we always store at least N * 2 elements
//  * Memory is not reclaimed
//
// Heavily based on https://github.com/rigtorp/HashMap by Erik Rigtorp
//  - modifications have been made in order to use the engines Asserts, avoid exceptions and document the algorithm more with comments
//  - license: 
/*
Copyright (c) 2017 Erik Rigtorp <erik@rigtorp.se>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
//
template <typename TKeyType, typename TValType, typename THashFn = std::hash<TKeyType>, typename TKeyEqu = std::equal_to<void>>
class lin_hash_map final
{
	//****************************************
	//**************
	// definitions
	//**************
	//****************************************
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
		explicit lin_iterator(TStorageType* const map) : m_Container(map) { advance_past_empty(); }
		explicit lin_iterator(TStorageType* const map, size_type const idx) : m_Container(map), m_Index(idx) {}

		// functionality
		//---------------
	public:
		lin_iterator& operator ++() 
		{
			++m_Index;
			advance_past_empty();
			return *this;
		}

		// accessors
		//-----------
		bool operator ==(lin_iterator const& other) const { return ((other.m_Container == m_Container) && (other.m_Index == m_Index)); }
		bool operator !=(lin_iterator const& other) const { return !(other == *this); }

		reference operator*() const { return m_Container->m_Buckets[m_Index]; }
		pointer operator->() const { return &m_Container->m_Buckets[m_Index]; };

		// utility
		//----------
	private:

		//---------------------------------
		// advance_past_empty
		//
		// move the iterator until it points at an occupied element
		//
		void advance_past_empty() 
		{
			while ((m_Index < m_Container->m_Buckets.size()) && key_equal()(m_Container->m_Buckets[m_Index].first, m_Container->m_EmptyKeyVal))
			{
				++m_Index;
			}
		}


		// Data
		///////

		TStorageType* m_Container;
		typename TStorageType::size_type m_Index = 0;
	};

	using iterator = lin_iterator<lin_hash_map, value_type>;
	using const_iterator = lin_iterator<lin_hash_map const, value_type const>;




	//***********************************************
	//**************
	// interface
	//**************
	//***********************************************

	// construct destruct
	//--------------------
	lin_hash_map(size_type const reqBucketCount, key_type const emptyKeyVal);
	lin_hash_map(lin_hash_map const& other, size_type const reqBucketCount);

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
	std::pair<iterator, bool> emplace(Args&&... args) { return emplace_impl(std::forward<Args>(args)...); }

	// remove an element at the iterator position
	void erase(iterator const it);

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

	


private:

	//****************************************
	//******************
	// implementation
	//******************
	//****************************************

	//---------------------------------
	// emplace_impl
	//
	// insert data into the container
	//
	template <typename TExtKeyType, typename... Args>
	std::pair<iterator, bool> emplace_impl(const TExtKeyType &key, Args &&... args)
	{
		ET_ASSERT(!key_equal()(m_EmptyKeyVal, key), "empty key shouldn't be used");

		reserve(m_Size + 1); // ensure we have enough space

		// first attempt placing data at the default index
		// we know we will find a free bucket in this loop because the bucket count is always greater than twice the element count
		for (size_t idx = key_to_idx(key);; idx = probe_next(idx)) 
		{
			if (key_equal()(m_Buckets[idx].first, m_EmptyKeyVal)) // if the element at this index is empty we can use it to store our data
			{
				m_Buckets[idx].second = mapped_type(std::forward<Args>(args)...);
				m_Buckets[idx].first = key;
				m_Size++;

				return { iterator(this, idx), true };
			}
			else if (key_equal()(m_Buckets[idx].first, key)) // if there is an element with the same key at this index, we won't insert
			{
				return { iterator(this, idx), false };
			}
		}
	}

	//---------------------------------
	// key_to_idx
	//
	// convert a key into a bucket index
	//
	template <typename TExtKeyType> 
	size_t key_to_idx(TExtKeyType const& key) const
	{
		// since the array size is a power of 2 we can use a mask instead of modulo - resulting in a mask that looks something like this 000001111111
		// any hash will get bits greater than the size of the buckets stripped
		size_t const mask = m_Buckets.size() - 1u; 
		return (hasher()(key) & mask);
	}

	size_t probe_next(size_t const idx) const;
	size_t diff(size_t a, size_t b) const;


	// Data
	///////

	key_type m_EmptyKeyVal;
	T_Buckets m_Buckets; // size will always be a power of 2
	size_t m_Size = 0u;
};


} // namespace core
} // namespace et


#include "linear_hash_map.inl"
