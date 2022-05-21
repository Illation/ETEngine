#include "stdafx.h"
#include "Context.h"

#include "RmlGlobal.h"
#include "RmlUtil.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtGUI/Content/GuiDocument.h>


namespace et {
namespace gui {


//=========
// Context 
//=========


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

//---------------
// Context::Init
//
void Context::Init(std::string const& name, ivec2 const dimensions)
{
	ET_ASSERT(RmlGlobal::IsInitialized());

	m_Context = ToPtr(Rml::CreateContext(name.c_str(), Rml::Vector2i(dimensions.x, dimensions.y)));
	ET_ASSERT(m_Context != nullptr, "Failed to create RmlUi context");
}

//--------------------
// Context::SetActive
//
void Context::SetActive(bool const isActive)
{
	if (m_Active != isActive)
	{
		m_Active = isActive;
		if (IsDocumentLoaded())
		{
			if (m_Active)
			{
				m_Document->Show();
			}
			else
			{
				m_Document->Hide();
			}
		}
	}
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
	ET_ASSERT(!IsDocumentLoaded());

	// this should automatically load all fonts referenced by the document into RML
	AssetPtr<GuiDocument> guiDocument = core::ResourceManager::Instance()->GetAssetData<GuiDocument>(documentId); 
	ET_ASSERT(guiDocument != nullptr);
	
	core::I_Asset const* const asset = guiDocument.GetAsset();

	m_Document = ToPtr(m_Context->LoadDocumentFromMemory(Rml::String(guiDocument->GetText(), guiDocument->GetLength()), 
		asset->GetPath() + asset->GetName()));

	if (m_Active)
	{
		m_Document->Show();
	}

	// we can let the guiDocument go out of scope because Rml now has it in memory
}

//-------------------------
// Context::UnloadDocument
//
void Context::UnloadDocument()
{
	m_Document->Close();
	m_Document = nullptr;
}

//-------------------------
// Context::UnloadDocument
//
void Context::Update()
{
	m_Context->Update();
}

//--------------------------------
// Context::ProcessKeyPressed
//
// #todo: Convert key events to text events for RML
//
bool Context::ProcessKeyPressed(E_KbdKey const key)
{
	return !(m_Context->ProcessKeyDown(RmlUtil::GetRmlKeyId(key), 0));
}

//---------------------------------
// Context::ProcessKeyReleased
//
bool Context::ProcessKeyReleased(E_KbdKey const key)
{
	return !(m_Context->ProcessKeyUp(RmlUtil::GetRmlKeyId(key), 0));
}

//----------------------------------
// Context::ProcessMousePressed
//
bool Context::ProcessMousePressed(E_MouseButton const button)
{
	return !(m_Context->ProcessMouseButtonDown(RmlUtil::GetRmlButtonIndex(button), 0));
}

//-----------------------------------
// Context::ProcessMouseReleased
//
bool Context::ProcessMouseReleased(E_MouseButton const button)
{
	return !(m_Context->ProcessMouseButtonUp(RmlUtil::GetRmlButtonIndex(button), 0));
}

//-------------------------------
// Context::ProcessMouseMove
//
bool Context::ProcessMouseMove(ivec2 const& mousePos)
{
	return !(m_Context->ProcessMouseMove(mousePos.x, mousePos.y, 0));
}

//-------------------------------------
// Context::ProcessMouseWheelDelta
//
bool Context::ProcessMouseWheelDelta(ivec2 const& mouseWheel)
{
	return (!m_Context->ProcessMouseWheel(static_cast<float>(mouseWheel.y), 0));
}


} // namespace gui
} // namespace et

