#pragma once


namespace et {
namespace core {


//=================
// Linear Hash Map
//=================


// for readability
#define LIN_HASH_MAP_TYPES typename TKeyType, typename TValType, typename THashFn, typename TKeyEqu
#define LIN_HASH_MAP_T lin_hash_map<TKeyType, TValType, THashFn, TKeyEqu>
#define LIN_HASH_MAP_TN typename LIN_HASH_MAP_T


// construct destruct
//////////////////////

//---------------------------------
// lin_hash_map::c-tor
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_T::lin_hash_map(size_type const reqBucketCount, key_type const emptyKeyVal)
	: m_EmptyKeyVal(emptyKeyVal)
{
	// find a bucket count larger or equal to the requirement that is a power of 2, and reserve that number of elements
	size_t pow2 = 2u; // we can start at 2, because a bucket count of 1 would resize before the first element gets inserted
	while (pow2 < reqBucketCount)
	{
		pow2 <<= 1u;
	}

	m_Buckets.resize(pow2, std::make_pair(emptyKeyVal, TValType()));
}

//---------------------------------
// lin_hash_map::c-tor
//
// Construct from another hash map
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_T::lin_hash_map(LIN_HASH_MAP_T const& other, size_type const reqBucketCount)
	: LIN_HASH_MAP_T(reqBucketCount, other.m_EmptyKeyVal)
{
	for (auto it = other.begin(); it != other.end(); ++it) 
	{
		insert(*it);
	}
}


// iterators
/////////////

//---------------------------------
// lin_hash_map::begin
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::iterator LIN_HASH_MAP_T::begin()
{
	return iterator(this);
}

//---------------------------------
// lin_hash_map::begin
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::const_iterator LIN_HASH_MAP_T::begin() const
{
	return const_iterator(this);
}

//---------------------------------
// lin_hash_map::cbegin
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::const_iterator LIN_HASH_MAP_T::cbegin() const
{
	return const_iterator(this);
}

//---------------------------------
// lin_hash_map::end
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::iterator LIN_HASH_MAP_T::end()
{
	return iterator(this, m_Buckets.size());
}

//---------------------------------
// lin_hash_map::end
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::const_iterator LIN_HASH_MAP_T::end() const
{
	return const_iterator(this, m_Buckets.size());
}

//---------------------------------
// lin_hash_map::cend
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::const_iterator LIN_HASH_MAP_T::cend() const
{
	return const_iterator(this, m_Buckets.size());
}


// accessors
/////////////

//---------------------------------
// lin_hash_map::at
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::mapped_type& LIN_HASH_MAP_T::at(key_type const& key)
{
	iterator const it = find(key);
	if (it != cend())
	{
		return it->second;
	}

	ET_WARNING("out of range");
	return *static_cast<mapped_type*>(nullptr);
}

//---------------------------------
// lin_hash_map::at
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::mapped_type const& LIN_HASH_MAP_T::at( key_type const& key) const
{
	const_iterator const it = find(key);
	if (it != cend())
	{
		return it->second;
	}

	ET_WARNING("out of range");
	return *static_cast<mapped_type const*>(nullptr);
}

//---------------------------------
// lin_hash_map:: []
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::mapped_type& LIN_HASH_MAP_T::operator []( key_type const& key)
{
	return at_impl(key);
}

//---------------------------------
// lin_hash_map::count
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::size_type LIN_HASH_MAP_T::count(key_type const& key) const
{
	return (find(key) == cend() ? 0 : 1);
}

//---------------------------------
// lin_hash_map::find
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::iterator LIN_HASH_MAP_T::find(key_type const& key) 
{
	ET_ASSERT(!key_equal()(m_EmptyKeyVal, key), "empty key shouldn't be used");

	// iterate over buckets starting at the bucket index of our key
	for (size_t idx = key_to_idx(key);; idx = probe_next(idx))
	{
		if (key_equal()(m_Buckets[idx].first, key)) // return the correct iterator if we find the matching key
		{
			return iterator(this, idx);
		}

		if (key_equal()(m_Buckets[idx].first, m_EmptyKeyVal)) // if we encounter an empty key we can stop the search, we know we won't find the element
		{
			return end();
		}
	}
}

//---------------------------------
// lin_hash_map::find
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::const_iterator LIN_HASH_MAP_T::find(key_type const& key) const
{
	ET_ASSERT(!key_equal()(m_EmptyKeyVal, key), "empty key shouldn't be used");

	for (size_t idx = key_to_idx(key);; idx = probe_next(idx))
	{
		if (key_equal()(m_Buckets[idx].first, key))
		{
			return const_iterator(this, idx);
		}

		if (key_equal()(m_Buckets[idx].first, m_EmptyKeyVal))
		{
			return end();
		}
	}
}


// capacity
////////////

//---------------------------------
// lin_hash_map::empty
//
template <LIN_HASH_MAP_TYPES>
bool LIN_HASH_MAP_T::empty() const
{
	return (size() == 0u);
}

//---------------------------------
// lin_hash_map::empty
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::size_type LIN_HASH_MAP_T::size() const
{
	return m_Size;
}

//---------------------------------
// lin_hash_map::max_size
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::size_type LIN_HASH_MAP_T::max_size() const
{
	return std::numeric_limits<size_type>::max();
}


// buckets
///////////

//---------------------------------
// lin_hash_map::empty
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::size_type LIN_HASH_MAP_T::bucket_count() const noexcept
{
	return m_Buckets.size();
}

//---------------------------------
// lin_hash_map::max_size
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::size_type LIN_HASH_MAP_T::max_bucket_count() const noexcept
{
	return std::numeric_limits<size_type>::max();
}


// functionality 
/////////////////

//---------------------------------
// lin_hash_map::insert
//
template <LIN_HASH_MAP_TYPES>
std::pair<LIN_HASH_MAP_TN::iterator, bool> LIN_HASH_MAP_T::insert(value_type const& value) 
{
	return emplace_impl(value.first, value.second);
}

//---------------------------------
// lin_hash_map::insert
//
template <LIN_HASH_MAP_TYPES>
std::pair<LIN_HASH_MAP_TN::iterator, bool> LIN_HASH_MAP_T::insert(value_type&& moving_value) 
{
	return emplace_impl(moving_value.first, std::move(moving_value.second));
}

//---------------------------------
// lin_hash_map::erase
//
template <LIN_HASH_MAP_TYPES>
void LIN_HASH_MAP_T::erase(iterator const it)
{
	size_t bucket = it.m_Index;
	for (size_t idx = probe_next(bucket);; idx = probe_next(idx)) // iterate buckets starting at the next element
	{
		if (key_equal()(m_Buckets[idx].first, m_EmptyKeyVal)) // we find the first empty bucket and return once we found it
		{
			m_Buckets[bucket].first = m_EmptyKeyVal;
			m_Size--;
			return;
		}

		// for any element that isn't empty we find it's ideal position
		// and swap if it's ideal position is closer to the bucket than it's current position
		// this ensures that elements are kept fast to find
		size_t ideal = key_to_idx(m_Buckets[idx].first); 
		if (diff(bucket, ideal) < diff(idx, ideal)) 
		{
			m_Buckets[bucket] = m_Buckets[idx]; 
			bucket = idx;
		}
	}
}

//---------------------------------
// lin_hash_map::erase
//
template <LIN_HASH_MAP_TYPES>
LIN_HASH_MAP_TN::size_type LIN_HASH_MAP_T::erase(key_type const& key)
{
	auto it = find(key);
	if (it != cend())
	{
		erase(it);
		return 1;
	}

	return 0;
}

//---------------------------------
// lin_hash_map::clear
//
template <LIN_HASH_MAP_TYPES>
void LIN_HASH_MAP_T::clear()
{
	LIN_HASH_MAP_T other(bucket_count(), m_EmptyKeyVal);
	swap(other);
}

//---------------------------------
// lin_hash_map::swap
//
template <LIN_HASH_MAP_TYPES>
void LIN_HASH_MAP_T::swap(LIN_HASH_MAP_T& other)
{
	std::swap(m_Buckets, other.m_Buckets);
	std::swap(m_Size, other.m_Size);
	std::swap(m_EmptyKeyVal, other.m_EmptyKeyVal);
}


// hashing 
///////////

//---------------------------------
// lin_hash_map::rehash
//
template <LIN_HASH_MAP_TYPES>
void LIN_HASH_MAP_T::rehash(size_type const bucketCount)
{
	size_type const new_count = std::max(bucketCount, size() * 2u);
	LIN_HASH_MAP_T other(*this, new_count);
	swap(other);
}

//---------------------------------
// lin_hash_map::reserve
//
template <LIN_HASH_MAP_TYPES>
void LIN_HASH_MAP_T::reserve(size_type const numElements)
{
	size_type const req_elem = numElements * 2u;
	if (req_elem > m_Buckets.size())
	{
		rehash(req_elem);
	}
}


// implementation 
//////////////////

//---------------------------------
// probe_next
//
// find the next index to check
//
template <LIN_HASH_MAP_TYPES>
size_t LIN_HASH_MAP_T::probe_next(size_t const idx) const
{
	size_t const mask = m_Buckets.size() - 1u;
	return ((idx + 1u) & mask);
}

//---------------------------------
// diff
//
// distance between two indices given the probing mask - accounts for wrapping around
//
template <LIN_HASH_MAP_TYPES>
size_t LIN_HASH_MAP_T::diff(size_t a, size_t b) const
{
	size_t const mask = m_Buckets.size() - 1u;
	return ((m_Buckets.size() + (a - b)) & mask);
}


} // namespace core
} // namespace et
