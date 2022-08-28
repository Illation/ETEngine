#pragma once
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/DataModelHandle.h>

#include <EtCore/Input/KeyCodes.h>

#include <EtRHI/GraphicsContext/ViewportEvents.h>

#include "DataModel.h"


// fwd
namespace Rml {
	class Context;
	class ElementDocument;
}


namespace et {
namespace gui {


class GuiDocument;


//---------------------------------
// Context
//
// Context for RML UI documents
//  - for now all contexts are screen space and the same size as the viewport
//
class Context
{
	struct Document final
	{
		Document(core::HashString const id, Ptr<Rml::ElementDocument> const doc, RefPtr<I_DataModel> const dataModel, core::HashString const modelId) 
			: m_Id(id), m_Document(doc), m_DataModel(dataModel), m_DataModelId(modelId) {}

		// Data
		///////

		core::HashString m_Id;

		Ptr<Rml::ElementDocument> m_Document;

		RefPtr<I_DataModel> m_DataModel;
		core::HashString m_DataModelId;

		bool m_IsActive = true;
	};

	typedef std::vector<Document> T_Documents;

public:
	// construct destruct
	//--------------------
	Context() = default;
	~Context();

	void Init(std::string const& name, ivec2 const dimensions);
	void Deinit();

	// functionality
	//---------------
	void SetDimensions(ivec2 const dimensions);

	void LoadDocument(core::HashString const documentId);
	void UnloadDocument(core::HashString const documentId);
	void SetDocumentActive(core::HashString const id, bool const isActive);

	void Update();
	void Render();

	bool ProcessKeyPressed(Rml::Input::KeyIdentifier const key, int32 const rmlModifier);
	bool ProcessKeyReleased(Rml::Input::KeyIdentifier const key, int32 const rmlModifier);
	bool ProcessMousePressed(int32 const button, int32 const rmlModifier);
	bool ProcessMouseReleased(int32 const button, int32 const rmlModifier);
	bool ProcessMouseMove(ivec2 const& mousePos, int32 const rmlModifier);
	bool ProcessMouseWheelDelta(ivec2 const& mouseWheel, int32 const rmlModifier);
	bool ProcessTextInput(Rml::Character const character);

	// accessors
	//-----------
	ivec2 GetDimensions() const;
	Rml::Context* GetImpl() { return m_Context.Get(); }

	bool HasActiveDocuments() const { return m_ActiveDocuments > 0; }
	bool IsDocumentActive(core::HashString const id) const;
	Rml::ElementDocument* GetDocument(core::HashString const id);
	size_t GetDocumentCount() const { return m_Documents.size(); }
	core::HashString GetDocumentId(size_t const docIdx) const { return m_Documents[docIdx].m_Id; }
	WeakPtr<I_DataModel> GetDataModel(core::HashString const documentId);

	// utility
	//---------
private:
	T_Documents::iterator GetDoc(core::HashString const id);
	T_Documents::const_iterator GetDoc(core::HashString const id) const;


	// Data
	///////

	Ptr<Rml::Context> m_Context;
	T_Documents m_Documents;
	size_t m_ActiveDocuments = 0u;
};


} // namespace gui
} // namespace et

