#pragma once
#include <RmlUi/Core/SystemInterface.h>

#include <EtCore/Input/CursorShapes.h>
#include <EtCore/Input/ClipboardControllerInterface.h>


namespace et {
namespace gui {


//---------------------------------
// RmlSystemInterface
//
// Implementation of RmlUi's system interface
//
class RmlSystemInterface final : public Rml::SystemInterface
{
public:
	// construct destruct
	//--------------------
	RmlSystemInterface() : Rml::SystemInterface() {}
	~RmlSystemInterface() = default;

	// functionality
	//---------------
	void SetCursorShapeManager(Ptr<core::I_CursorShapeManager> const cursorMan) { m_CursorShapeManager = cursorMan; }
	void SetClipboardController(Ptr<core::I_ClipboardController> const clipboardController) { m_ClipboardController = clipboardController; }

	// interface implementation
	//--------------------------
	double GetElapsedTime() override;

	bool LogMessage(Rml::Log::Type type, Rml::String const& message) override;

	void SetMouseCursor(Rml::String const& cursor_name) override;

	void SetClipboardText(Rml::String const& text) override;
	void GetClipboardText(Rml::String& outText) override;

	// Data
	///////

private:

	// if set, will take responsibility for changing the cursor shape
	Ptr<core::I_CursorShapeManager> m_CursorShapeManager;
	Ptr<core::I_ClipboardController> m_ClipboardController;
};


} // namespace gui
} // namespace et

