#include "stdafx.h"
#include "Context.h"

#include "RmlGlobal.h"
#include "GuiDocument.h"
#include "RmlUtil.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>

#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/ResourceManager.h>


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
	ET_ASSERT(GetDoc(documentId) == m_Documents.cend());

	// this should automatically load all fonts referenced by the document into RML
	AssetPtr<GuiDocument> guiDocument = core::ResourceManager::Instance()->GetAssetData<GuiDocument>(documentId); 
	ET_ASSERT(guiDocument != nullptr);
	
	core::I_Asset const* const asset = guiDocument.GetAsset();

	Rml::String const docText(guiDocument->GetText(), guiDocument->GetLength());
	m_Documents.emplace_back(documentId, ToPtr(m_Context->LoadDocumentFromMemory(docText, asset->GetPath() + asset->GetName())));
	m_Documents.back().m_Document->Show();
	m_ActiveDocuments++;

	// we can let the guiDocument go out of scope because Rml now has it in memory
}

//-------------------------
// Context::UnloadDocument
//
void Context::UnloadDocument(core::HashString const documentId)
{
	T_Documents::iterator const foundIt = GetDoc(documentId);
	ET_ASSERT(foundIt != m_Documents.cend());

	foundIt->m_Document->Close();
	if (foundIt->m_IsActive)
	{
		m_ActiveDocuments--;
	}

	core::RemoveSwap(m_Documents, foundIt);
}

//-------------------------
// Context::UnloadDocument
//
void Context::SetDocumentActive(core::HashString const id, bool const isActive)
{
	auto const foundIt = GetDoc(id);
	ET_ASSERT(foundIt != m_Documents.cend());

	if (foundIt->m_IsActive != isActive)
	{
		foundIt->m_IsActive = isActive;
		if (isActive)
		{
			foundIt->m_Document->Show();
		}
		else
		{
			foundIt->m_Document->Hide();
		}
	}
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

//-------------------------------------
// Context::ProcessTextInput
//
ivec2 Context::GetDimensions() const
{
	ET_ASSERT(m_Context != nullptr);
	return RmlUtil::ToEtm(m_Context->GetDimensions());
}

//-------------------------------------
// Context::IsDocumentActive
//
bool Context::IsDocumentActive(core::HashString const id) const
{
	T_Documents::const_iterator const foundIt = GetDoc(id);
	if (foundIt != m_Documents.cend())
	{
		return foundIt->m_IsActive;
	}

	return false;
}

//-------------------------------------
// Context::GetDocument
//
Rml::ElementDocument* Context::GetDocument(core::HashString const id)
{
	auto const found = GetDoc(id);
	ET_ASSERT(found != m_Documents.cend());
	return found->m_Document.Get();
}

//-------------------------------------
// Context::GetDoc
//
Context::T_Documents::iterator Context::GetDoc(core::HashString const id)
{
	return std::find_if(m_Documents.begin(), m_Documents.end(), [id](Document const& doc)
		{
			return (doc.m_Id == id);
		});
}

//-------------------------------------
// Context::GetDoc
//
Context::T_Documents::const_iterator Context::GetDoc(core::HashString const id) const
{
	return std::find_if(m_Documents.cbegin(), m_Documents.cend(), [id](Document const& doc)
		{
			return (doc.m_Id == id);
		});
}


} // namespace gui
} // namespace et

