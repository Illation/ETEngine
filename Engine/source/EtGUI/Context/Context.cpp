#include "stdafx.h"
#include "Context.h"

#include "RmlGlobal.h"

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

//--------------------------
// Context::CreateDataModel
//
Rml::DataModelConstructor Context::CreateDataModel(std::string const& modelName)
{
	return m_Context->CreateDataModel(modelName);
}

//---------------------------
// Context::DestroyDataModel
//
bool Context::DestroyDataModel(std::string const& modelName)
{
	return m_Context->RemoveDataModel(modelName);
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

	Rml::String const docText(guiDocument->GetText(), guiDocument->GetLength());
	m_Document = ToPtr(m_Context->LoadDocumentFromMemory(docText, asset->GetPath() + asset->GetName()));

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

//-------------------------
// Context::UnloadDocument
//
void Context::Render()
{
	m_Context->Render();
}

//--------------------------------
// Context::ProcessKeyPressed
//
bool Context::ProcessKeyPressed(Rml::Input::KeyIdentifier const key, int32 const rmlModifier)
{
	return !(m_Context->ProcessKeyDown(key, rmlModifier));
}

//---------------------------------
// Context::ProcessKeyReleased
//
bool Context::ProcessKeyReleased(Rml::Input::KeyIdentifier const key, int32 const rmlModifier)
{
	return !(m_Context->ProcessKeyUp(key, rmlModifier));
}

//----------------------------------
// Context::ProcessMousePressed
//
bool Context::ProcessMousePressed(int32 const button, int32 const rmlModifier)
{
	return !(m_Context->ProcessMouseButtonDown(button, rmlModifier));
}

//-----------------------------------
// Context::ProcessMouseReleased
//
bool Context::ProcessMouseReleased(int32 const button, int32 const rmlModifier)
{
	return !(m_Context->ProcessMouseButtonUp(button, rmlModifier));
}

//-------------------------------
// Context::ProcessMouseMove
//
bool Context::ProcessMouseMove(ivec2 const& mousePos, int32 const rmlModifier)
{
	return !(m_Context->ProcessMouseMove(mousePos.x, mousePos.y, rmlModifier));
}

//-------------------------------------
// Context::ProcessMouseWheelDelta
//
bool Context::ProcessMouseWheelDelta(ivec2 const& mouseWheel, int32 const rmlModifier)
{
	return !(m_Context->ProcessMouseWheel(static_cast<float>(mouseWheel.y), rmlModifier));
}

//-------------------------------------
// Context::ProcessTextInput
//
bool Context::ProcessTextInput(Rml::Character const character)
{
	return !(m_Context->ProcessTextInput(character));
}


} // namespace gui
} // namespace et

