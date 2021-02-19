#include "stdafx.h"
#include "ColladaDom.h"


namespace et {
namespace edit {

namespace dae {


//=========
// Source
//=========


//---------------
// Source::c-tor
//
Source::Source(core::HashString const id, core::HashString const dataId, E_Type const type, Accessor const* const accessor, core::XML::Element const& el) 
	: m_Id(id), 
	m_DataId(dataId), 
	m_Type(type), 
	m_CommonAccessor(accessor), 
	m_Element(&el)
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

	m_Element = other.m_Element;
	m_IsParsed = other.m_IsParsed;
	
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

	m_Element = other.m_Element;
	m_IsParsed = other.m_IsParsed;

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