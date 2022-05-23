#pragma once
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/DataModelHandle.h>

#include <EtCore/Input/KeyCodes.h>

#include <EtRendering/GraphicsContext/ViewportEvents.h>


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
public:
	// construct destruct
	//--------------------
	Context() = default;
	~Context();

	void Init(std::string const& name, ivec2 const dimensions);

	// functionality
	//---------------
	void SetActive(bool const isActive);
	void SetDimensions(ivec2 const dimensions);

	Rml::DataModelConstructor CreateDataModel(std::string const& modelName);
	bool DestroyDataModel(std::string const& modelName);

	void LoadDocument(core::HashString const documentId);
	void UnloadDocument();

	void Update();

	bool ProcessKeyPressed(Rml::Input::KeyIdentifier const key, int32 const rmlModifier);
	bool ProcessKeyReleased(Rml::Input::KeyIdentifier const key, int32 const rmlModifier);
	bool ProcessMousePressed(int32 const button, int32 const rmlModifier);
	bool ProcessMouseReleased(int32 const button, int32 const rmlModifier);
	bool ProcessMouseMove(ivec2 const& mousePos, int32 const rmlModifier);
	bool ProcessMouseWheelDelta(ivec2 const& mouseWheel, int32 const rmlModifier);
	bool ProcessTextInput(Rml::Character const character);

	// accessors
	//-----------
	bool IsActive() const { return m_Active; }
	bool IsDocumentLoaded() const { return (m_Document != nullptr); }

	// Data
	///////

private:
	Ptr<Rml::Context> m_Context;
	bool m_Active = true;
	Ptr<Rml::ElementDocument> m_Document; 
};


} // namespace gui
} // namespace et

