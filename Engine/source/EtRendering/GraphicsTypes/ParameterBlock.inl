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
	return *reinterpret_cast<TParamType const*>(block + offset);
}

//---------------------------------
// Write
//
template<typename TParamType, typename std::enable_if_t<!std::is_pointer<TParamType>::value>*>
void Write(T_ParameterBlock const block, size_t const offset, TParamType const& param)
{
	memcpy(block + offset, &param, sizeof(TParamType));
}

//---------------------------------
// Write
//
template<typename TParamType, typename std::enable_if_t<std::is_pointer<TParamType>::value>*>
void Write(T_ParameterBlock const block, size_t const offset, TParamType const& param)
{
	memcpy(block + offset, &param, sizeof(TParamType));
}


} // namespace parameters
} // namespace render

