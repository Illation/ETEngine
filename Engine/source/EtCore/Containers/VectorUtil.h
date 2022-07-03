#pragma once


namespace et {
namespace core {


//-------------------------
// RemoveSwap
//
// Fast-erase function for vectors by swapping the element with the last element of the vector
//
template <typename TDataType, typename TIteratorType>
void RemoveSwap(std::vector<TDataType>& vec, TIteratorType const it)
{
	auto const backIt = std::prev(vec.end());
	if (it != backIt)
	{
		std::iter_swap(it, backIt);
	}

	vec.pop_back();
}


} // namespace core
} // namespace et

