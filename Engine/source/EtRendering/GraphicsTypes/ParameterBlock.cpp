#include "stdafx.h"
#include "ParameterBlock.h"
#include "TextureData.h"


namespace et {
namespace render {
namespace parameters {


//=================
// Parameter Block
//=================


//---------------------------------
// GetSize
//
size_t GetSize(E_ParamType const type)
{
	switch (type)
	{
	case E_ParamType::Texture2D: return sizeof(TextureData*);
	case E_ParamType::Texture3D: return sizeof(TextureData*);
	case E_ParamType::TextureCube: return sizeof(TextureData*);
	case E_ParamType::TextureShadow: return sizeof(TextureData*);
	case E_ParamType::Matrix4x4: return sizeof(mat4);
	case E_ParamType::Matrix3x3: return sizeof(mat3);
	case E_ParamType::Vector4: return sizeof(vec4);
	case E_ParamType::Vector3: return sizeof(vec3);
	case E_ParamType::Vector2: return sizeof(vec2);
	case E_ParamType::UInt: return sizeof(uint32);
	case E_ParamType::Int: return sizeof(int32);
	case E_ParamType::Float: return sizeof(float);
	case E_ParamType::Boolean: return sizeof(bool);
	}

	ET_ERROR("Unhandled parameter type!");
	return 0u;
}

//---------------------------------
// GetTypeId
//
rttr::type GetTypeId(E_ParamType const type)
{
	switch (type)
	{
	case E_ParamType::Texture2D: return rttr::type::get<TextureData const*>();
	case E_ParamType::Texture3D: return rttr::type::get<TextureData const*>();
	case E_ParamType::TextureCube: return rttr::type::get<TextureData const*>();
	case E_ParamType::TextureShadow: return rttr::type::get<TextureData const*>();
	case E_ParamType::Matrix4x4: return rttr::type::get<mat4>();
	case E_ParamType::Matrix3x3: return rttr::type::get<mat3>();
	case E_ParamType::Vector4: return rttr::type::get<vec4>();
	case E_ParamType::Vector3: return rttr::type::get<vec3>();
	case E_ParamType::Vector2: return rttr::type::get<vec2>();
	case E_ParamType::UInt: return rttr::type::get<uint32>();
	case E_ParamType::Int: return rttr::type::get<int32>();
	case E_ParamType::Float: return rttr::type::get<float>();
	case E_ParamType::Boolean: return rttr::type::get<bool>();
	}

	ET_ERROR("Unhandled parameter type!");
	return rttr::type::get<std::nullptr_t>();
}

//---------------------------------
// MatchesTexture
//
bool MatchesTexture(E_ParamType const param, E_TextureType const texture)
{
	switch (param)
	{
	case E_ParamType::Texture2D: return (texture == E_TextureType::Texture2D);
	case E_ParamType::Texture3D: return (texture == E_TextureType::Texture3D);
	case E_ParamType::TextureCube: return (texture == E_TextureType::CubeMap);
	case E_ParamType::TextureShadow: return (texture == E_TextureType::Texture2D);
	}

	return false;
}

//---------------------------------
// CreateBlock
//
T_ParameterBlock CreateBlock(size_t const size)
{
	return new uint8[size];
}

//---------------------------------
// DestroyBlock
//
void DestroyBlock(T_ParameterBlock& block)
{
	delete[] block;
	block = nullptr;
}

//---------------------------------
// CopyBlockData
//
// Fill in the target block based on the source block
//
void CopyBlockData(T_ConstParameterBlock const source, T_ParameterBlock const target, size_t const blockSize)
{
	memcpy(target, source, blockSize);
}

//---------------------------------
// Compare
//
// Check if two parameter blocks hold the same data
//
bool Compare(T_ConstParameterBlock const lhs, T_ConstParameterBlock const rhs, size_t const offset, E_ParamType const type)
{
	return (memcmp(lhs + offset, rhs + offset, GetSize(type)) == 0);
}


} // namespace parameters
} // namespace render
} // namespace et
