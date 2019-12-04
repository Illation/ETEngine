#pragma once


namespace render {
namespace parameters {


//=================
// Parameter Block
//=================


//---------------------------------
// Read
//
template<typename TParamType>
TParamType const& Read(T_ConstParameterBlock const block, size_t const offset)
{
	return reinterpret_cast<TParamType>(block[offset]);
}

//---------------------------------
// Write
//
template<typename TParamType>
void Write(T_ParameterBlock const block, size_t const offset, TParamType const& param)
{
	memcpy(block + offset, reinterpret_cast<uint8*>(&param), sizeof(TParamType));
}


} // namespace parameters
} // namespace render

