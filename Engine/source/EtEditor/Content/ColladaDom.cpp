#include "stdafx.h"
#include "ColladaDom.h"


namespace et {
namespace edit {

namespace dae {


//========
// Asset
//========


//---------------------
// Asset::Get3DIndices
//
// based on up axis, how to index 3D vector components to convert into the engines coordinate system
//
ivec3 Asset::Get3DIndices() const
{
	switch (m_UpAxis)
	{
	case E_Axis::X: return ivec3(1, 0, 2);
	case E_Axis::Z: return ivec3(0, 2, 1);

	default:
		ET_ASSERT(false, "unhandled axis");
	case E_Axis::Y: return ivec3(0, 1, 2);
	}
}

//-----------------------------
// Asset::Get3DAxisMultipliers
//
// based on up axis, inversion of vector coordinates
//
vec3 Asset::Get3DAxisMultipliers() const
{
	switch (m_UpAxis)
	{
	case E_Axis::X: return vec3(-1.f, 1.f, -1.f);
	case E_Axis::Z: return vec3(1.f, 1.f, 1.f);

	default:
		ET_ASSERT(false, "unhandled axis");
	case E_Axis::Y: return vec3(1.f, 1.f, -1.f);
	}
}


//=========
// Source
//=========


//---------------------
// Source::GetTypeSize
//
// in bytes
//
uint8 Source::GetTypeSize(E_Type const type) 
{
	switch (type)
	{
	case E_Type::IDREF: 
	case E_Type::Name: 
		return sizeof(core::HashString);

	case E_Type::Bool: return sizeof(bool);
	case E_Type::Float: return sizeof(float);
	case E_Type::Int: return sizeof(int64);
	}

	ET_ASSERT(false, "unhandled type");
	return 0u;
}

//---------------
// Source::c-tor
//
Source::Source(core::HashString const id, 
	core::HashString const dataId, 
	E_Type const type, 
	Accessor const* const accessor, 
	core::XML::Element const& dataEl)
	: m_Id(id)
	, m_DataId(dataId)
	, m_Type(type)
	, m_CommonAccessor(accessor)
	, m_DataEl(&dataEl)
{ }

//---------------
// Source::copy
//
Source::Source(Source const& other)
{
	*this = other;
}

//------------
// Source:: = 
//
Source& Source::operator=(Source const& other)
{
	m_Id = other.m_Id;
	m_DataId = other.m_DataId;
	m_Type = other.m_Type;
	m_Buffer = other.m_Buffer;

	delete m_CommonAccessor;
	if (other.m_CommonAccessor != nullptr)
	{
		m_CommonAccessor = new Accessor(*other.m_CommonAccessor);
	}
	else
	{
		m_CommonAccessor = nullptr;
	}

	m_DataEl = other.m_DataEl;
	m_IsResolved = other.m_IsResolved;
	
	return *this;
}

//---------------
// Source::move
//
Source::Source(Source&& other)
{
	*this = other;
}

//---------------
// Source::move
//
Source& Source::operator=(Source&& other)
{
	m_Id = other.m_Id;
	m_DataId = other.m_DataId;
	m_Type = other.m_Type;
	m_Buffer = std::move(other.m_Buffer);

	m_CommonAccessor = other.m_CommonAccessor;
	other.m_CommonAccessor = nullptr;

	m_DataEl = other.m_DataEl;
	m_IsResolved = other.m_IsResolved;

	return *this;
}

//---------------
// Source::d-tor
//
Source::~Source()
{
	delete m_CommonAccessor;
	m_CommonAccessor = nullptr;
}


//========
// Input
//========


size_t const Input::s_InvalidIndex = std::numeric_limits<size_t>::max();



} // namespace dae

} // namespace edit
} // namespace et