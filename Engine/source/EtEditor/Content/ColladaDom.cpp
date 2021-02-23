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


//===========
// Instance
//===========


std::vector<core::HashString> const Instance::s_XmlIds = std::vector<core::HashString>({
	core::HashString("instance_visual_scene"),
	core::HashString("instance_node"),
	core::HashString("instance_geometry") });


//----------------------------
// Instance::GetTypeFromXmlId
//
Instance::E_Type Instance::GetTypeFromXmlId(core::HashString const id)
{
	auto const foundIt = std::find(s_XmlIds.cbegin(), s_XmlIds.cend(), id);
	if (foundIt != s_XmlIds.cend())
	{
		return static_cast<E_Type>(foundIt - s_XmlIds.cbegin());
	}

	return E_Type::None;
}

//----------------------------
// Instance::GetXmlIdFromType
//
core::HashString Instance::GetXmlIdFromType(E_Type const type)
{
	size_t const typeIdx = static_cast<size_t>(type);
	if (typeIdx < s_XmlIds.size())
	{
		return s_XmlIds[typeIdx];
	}

	ET_ASSERT(false, "unhandled type");
	return core::HashString();
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


//=======
// Node
//=======


//----------------------------
// Node::GetGeometryTransform
//
// recursively find a node with the given geometry instance and calculate a transformation matrix
//
bool Node::GetGeometryTransform(mat4& base, core::HashString const geometryId) const
{
	if (std::find_if(m_Instances.cbegin(), m_Instances.cend(), [geometryId](Instance const& inst)
		{
			return ((inst.m_Type == Instance::E_Type::Geometry) && (inst.m_Url == geometryId));
		}) != m_Instances.cend())
	{
		base = m_Transform;
		return true;
	}

	for (Node const& child : m_Children)
	{
		if (child.GetGeometryTransform(base, geometryId))
		{
			base = m_Transform * base;
			return true;
		}
	}

	return false;
}


} // namespace dae

} // namespace edit
} // namespace et