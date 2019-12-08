#include "stdafx.h"
#include "ParameterBlock.h"


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

	ET_ASSERT(false, "Unhandled parameter type!");
	return 0u;
}

//---------------------------------
// GetTypeId
//
std::type_info const& GetTypeId(E_ParamType const type)
{
	switch (type)
	{
	case E_ParamType::Texture2D: return typeid(TextureData const*);
	case E_ParamType::Texture3D: return typeid(TextureData const*);
	case E_ParamType::TextureCube: return typeid(TextureData const*);
	case E_ParamType::TextureShadow: return typeid(TextureData const*);
	case E_ParamType::Matrix4x4: return typeid(mat4);
	case E_ParamType::Matrix3x3: return typeid(mat3);
	case E_ParamType::Vector4: return typeid(vec4);
	case E_ParamType::Vector3: return typeid(vec3);
	case E_ParamType::Vector2: return typeid(vec2);
	case E_ParamType::UInt: return typeid(uint32);
	case E_ParamType::Int: return typeid(int32);
	case E_ParamType::Float: return typeid(float);
	case E_ParamType::Boolean: return typeid(bool);
	}

	ET_ASSERT(false, "Unhandled parameter type!");
	return typeid(nullptr);
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


} // namespace parameters
} // namespace render
