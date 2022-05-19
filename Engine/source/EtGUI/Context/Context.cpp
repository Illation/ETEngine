#include "stdafx.h"
#include "Context.h"

#include "RmlGlobal.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>


namespace et {
namespace gui {


//=========
// Context 
//=========


//---------------
// Context::Init
//
void Context::Init(std::string const& name, ivec2 const dimensions)
{
	ET_ASSERT(RmlGlobal::IsInitialized());

	m_Context = ToPtr(Rml::CreateContext(name.c_str(), Rml::Vector2i(dimensions.x, dimensions.y)));
	ET_ASSERT(m_Context != nullptr, "Failed to create RmlUi context");
}

//------------------------
// Context::SetDimensions
//
void Context::SetDimensions(ivec2 const dimensions)
{
	m_Context->SetDimensions(Rml::Vector2i(dimensions.x, dimensions.y));
}

//-----------------------
// Context::LoadDocument
//
void Context::LoadDocument(core::HashString const documentId)
{
	m_Document = documentId;
}

//-------------------------
// Context::UnloadDocument
//
void Context::UnloadDocument()
{
	m_Document.Reset();
}

//----------------
// Context::d-tor
//
Context::~Context()
{
	if (RmlGlobal::IsInitialized() && (m_Context != nullptr))
	{
		Rml::RemoveContext(m_Context->GetName());
	}
}


} // namespace gui
} // namespace et

