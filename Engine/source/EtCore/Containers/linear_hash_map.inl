#pragma once


namespace et {
namespace core {


//=================
// Linear Hash Map
//=================


//---------------------------------
// lin_hash_map::c-tor
//
template <typename TKeyType, typename TValType, typename THashFn, typename TKeyEqu>
lin_hash_map<TKeyType, TValType, THashFn, TKeyEqu>::lin_hash_map(size_type const bucketCount, key_type const emptyKeyVal)
	: m_EmptyKeyVal(emptyKeyVal)
{
	m_Buckets.resize(std::pow(bucketCount, 2u), std::make_pair(emptyKeyVal, TValType()));
}

//---------------------------------
// lin_hash_map::c-tor
//
// Construct from another hash map
//
template <typename TKeyType, typename TValType, typename THashFn, typename TKeyEqu>
lin_hash_map<TKeyType, TValType, THashFn, TKeyEqu>::lin_hash_map( lin_hash_map<TKeyType, TValType, THashFn, TKeyEqu> const& other, size_type const bucketCount)
	: lin_hash_map<TKeyType, TValType, THashFn, TKeyEqu>(bucketCount, other.m_EmptyKeyVal)
{
	for (auto it = other.begin(); it != other.end(); ++it) 
	{
		insert(*it);
	}
}


} // namespace core
} // namespace et
