#pragma once


namespace et {


//----------
// RefCount
//
// Utility that tracks strong and weak references to an object
//
struct RefCount final
{
	int32 m_References = 1u; // we always create this starting with one reference
	int32 m_WeakReferences = 0u;
};


} // namespace et
