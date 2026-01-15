#include "stdafx.h"
#include "Context.h"

#include "RmlGlobal.h"
#include "GuiDocument.h"
#include "RmlUtil.h"
#include "RmlDebuggerFwd.h"

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


Context::T_ContextMap Context::s_ContextWrapperLookup;


//----------------
// Context::Get
//
// Get the relevant context wrapper for an RML context
//
Context* Context::Get(Rml::Context const* const context) 
{
	T_ContextMap::iterator foundIt = s_ContextWrapperLookup.find(ToPtr(context));
	ET_ASSERT(foundIt != s_ContextWrapperLookup.cend(), "Rml Context shouldn't be created without using the engines wrapper!");

	return foundIt->second.Get();
}

//----------------
// Context::d-tor
//
Context::~Context()
{
	Deinit();
}

//---------------
// Context::Init
//
void Context::Init(std::string const& name, ivec2 const dimensions)
{
	ET_ASSERT(RmlGlobal::IsInitialized());

	m_Context = ToPtr(Rml::CreateContext(name.c_str(), Rml::Vector2i(dimensions.x, dimensions.y)));
	ET_ASSERT(m_Context != nullptr, "Failed to create RmlUi context");

	s_ContextWrapperLookup[m_Context] = ToPtr(this);
}

//-----------------
// Context::Deinit
//
void Context::Deinit()
{
	if (m_Context != nullptr)
	{
		T_ContextMap::const_iterator foundIt = s_ContextWrapperLookup.find(m_Context);
		ET_ASSERT(foundIt != s_ContextWrapperLookup.cend());

		s_ContextWrapperLookup.erase(foundIt);

		if (RmlGlobal::IsInitialized())
		{
			Rml::RemoveContext(m_Context->GetName());

#		if ET_CT_IS_ENABLED(ET_CT_RML_DEBUGGER)
			RmlGlobal::GetInstance()->OnContextDestroyed(m_Context.Get());
#		endif
		}
	}

	m_Context = nullptr;
	m_ActiveDocuments = 0u;
	m_Documents.clear();
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
	ET_ASSERT(GetDoc(documentId) == m_Documents.cend());

	// this should automatically load all fonts referenced by the document into RML
	AssetPtr<GuiDocument> guiDocument = core::ResourceManager::Instance()->GetAssetData<GuiDocument>(documentId); 
	ET_ASSERT(guiDocument != nullptr);
	
	GuiDocumentAsset const* const asset = static_cast<GuiDocumentAsset const*>(guiDocument.GetAsset());

	RefPtr<I_DataModel> const dataModel = 
		asset->m_DataModelId.IsEmpty() ? nullptr : RmlGlobal::GetInstance()->GetDataModelFactory().CreateModel(*m_Context, asset->m_DataModelId);

	Rml::String const docText(guiDocument->GetText(), guiDocument->GetLength());
	m_Documents.emplace_back(documentId, 
		ToPtr(m_Context->LoadDocumentFromMemory(docText, asset->GetPath() + asset->GetName())), 
		dataModel, 
		asset->m_DataModelId);
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

	if (foundIt->m_DataModel != nullptr)
	{
		std::string dataModelName;
		RmlGlobal::GetInstance()->GetDataModelFactory().GetModelName(foundIt->m_DataModelId, dataModelName);
		m_Context->RemoveDataModel(dataModelName);
	}

	foundIt->m_Document->Close();
	if (foundIt->m_IsActive)
	{
		m_ActiveDocuments--;
	}

	core::RemoveSwap(m_Documents, foundIt);
}

//----------------------------
// Context::SetDocumentActive
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

//-----------------
// Context::Update
//
void Context::Update()
{
	m_Context->Update();
}

//-----------------
// Context::Render
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
	if (button == 0)
	{
		m_MouseUpListeners.insert(m_MouseUpListeners.end(), m_MouseUpListenersToAdd.begin(), m_MouseUpListenersToAdd.end());
		m_MouseUpListenersToAdd.clear();

		m_IteratingListeners = true;
		for (Ptr<I_MouseUpListener> listener : m_MouseUpListeners)
		{
			if (std::find(m_MouseUpListenersToRemove.begin(), m_MouseUpListenersToRemove.end(), listener) == m_MouseUpListenersToRemove.end())
			{
				if (listener->OnMouseUp())
				{
					return !m_Context->IsMouseInteracting();
				}
			}
		}

		m_IteratingListeners = false;

		m_MouseUpListeners.erase(std::remove_if(m_MouseUpListeners.begin(), m_MouseUpListeners.end(),
			[this](Ptr<I_MouseUpListener> const listener)
			{
				return std::find(m_MouseUpListenersToRemove.begin(), m_MouseUpListenersToRemove.end(), listener) != m_MouseUpListenersToRemove.end();
			}), m_MouseUpListeners.end());
		m_MouseUpListenersToRemove.clear();
	}

	return !(m_Context->ProcessMouseButtonUp(button, rmlModifier));
}

//-------------------------------
// Context::ProcessMouseMove
//
bool Context::ProcessMouseMove(ivec2 const& mousePos, int32 const rmlModifier)
{
	m_MousePos = mousePos;
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
// Context::ProcessMouseWheelDelta
//
bool Context::ProcessMouseLeave()
{
	return !(m_Context->ProcessMouseLeave());
}

//-------------------------------------
// Context::ProcessTextInput
//
bool Context::ProcessTextInput(Rml::Character const character)
{
	return !(m_Context->ProcessTextInput(character));
}

//-------------------------------------
// Context::RegisterMouseUpListener
//
void Context::RegisterMouseUpListener(Ptr<I_MouseUpListener> const listener)
{
	if (m_IteratingListeners)
	{
		core::PushUnique(m_MouseUpListenersToAdd, listener);
	}
	else
	{
		ET_ASSERT(std::find(m_MouseUpListeners.cbegin(), m_MouseUpListeners.cend(), listener) == m_MouseUpListeners.cend());
		m_MouseUpListeners.push_back(listener);
	}
}

//-------------------------------------
// Context::UnregisterMouseUpListener
//
void Context::UnregisterMouseUpListener(Ptr<I_MouseUpListener> const listener)
{
	if (m_IteratingListeners)
	{
		core::PushUnique(m_MouseUpListenersToRemove, listener);
	}
	else
	{
		T_MouseUpListeners::iterator foundIt = std::find(m_MouseUpListeners.begin(), m_MouseUpListeners.end(), listener);
		ET_ASSERT(foundIt != m_MouseUpListeners.cend());
		core::RemoveSwap(m_MouseUpListeners, foundIt);
	}
}

//-------------------------------------
// Context::GetDimensions
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
// Context::GetDataModel
//
WeakPtr<I_DataModel> Context::GetDataModel(core::HashString const documentId)
{
	T_Documents::iterator const foundIt = GetDoc(documentId);
	ET_ASSERT(foundIt != m_Documents.cend());

	return foundIt->m_DataModel;
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

