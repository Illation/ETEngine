#pragma once
#include <EtRHI/GraphicsContext/GraphicsTypes.h>


namespace et {
namespace rhi {


//---------------------------------
// T_ParameterBlock
//
// in order to make parameter data cache friendly it is maintained as continuous array data
//
typedef uint8* T_ParameterBlock;
typedef uint8 const* T_ConstParameterBlock;


//---------------------------------
// UniformParam
//
// access information for a uniform parameter within a parameterBlock
//
struct UniformParam
{
	T_UniformLoc location;
	E_ParamType type;
	size_t offset;
};


//---------------------------------
// parameters
//
// utility functionality to deal with parameter data
//
namespace parameters {


size_t GetSize(E_ParamType const type);
rttr::type GetTypeId(E_ParamType const type);
bool MatchesTexture(E_ParamType const param, E_TextureType const texture);

T_ParameterBlock CreateBlock(size_t const size);
void DestroyBlock(T_ParameterBlock& block);

void CopyBlockData(T_ConstParameterBlock const source, T_ParameterBlock const target, size_t const blockSize);

bool Compare(T_ConstParameterBlock const lhs, T_ConstParameterBlock const rhs, size_t const offset, E_ParamType const type);

template<typename TParamType>
TParamType const& Read(T_ConstParameterBlock const block, size_t const offset);

template<typename TParamType, typename std::enable_if_t<!std::is_pointer<TParamType>::value>* = 0>
void Write(T_ParameterBlock const block, size_t const offset, TParamType const& param);

template<typename TParamType, typename std::enable_if_t<std::is_pointer<TParamType>::value>* = 0>
void Write(T_ParameterBlock const block, size_t const offset, TParamType const& param);


} // namespace parameters


} // namespace rhi
} // namespace et


#include "ParameterBlock.inl"
