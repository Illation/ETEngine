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
