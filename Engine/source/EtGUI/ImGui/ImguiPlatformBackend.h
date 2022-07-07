#pragma once
#include <imgui/imgui.h>

#ifndef IMGUI_DISABLE

#include <EtCore/Input/RawInputListener.h>
#include <EtCore/Util/CursorShapes.h>
#include <EtCore/Util/ClipboardControllerInterface.h>

#include <EtRendering/GraphicsContext/GraphicsContext.h>
#include <EtRendering/GraphicsContext/Viewport.h>


namespace et {
namespace gui {


//-----------------
// ImguiPlatformBackend
//
// Functionality for providing input for imgui
//
class ImguiPlatformBackend final : public core::I_RawInputListener
{
	// static functionality
	//----------------------
public:
	static ImguiPlatformBackend* AccessFromIO();

	// construct destruct
	//--------------------
	ImguiPlatformBackend() : core::I_RawInputListener() {}

	void Init(Ptr<core::I_CursorShapeManager> const cursorManager,
		Ptr<core::I_ClipboardController> const clipboardController,
		Ptr<render::Viewport> const viewport);
	void Deinit();

	// functionality
	//---------------
	void Update();

	// accessors
	//-----------
	render::Viewport* GetViewport() { return m_Viewport.Get(); }

	// interface
	//-----------
protected:
	int8 GetPriority() const override { return 2; }
	bool ProcessKeyPressed(E_KbdKey const key, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessKeyReleased(E_KbdKey const key, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessMousePressed(E_MouseButton const button, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessMouseReleased(E_MouseButton const button, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessMouseMove(ivec2 const& mousePos, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessMouseWheelDelta(ivec2 const& mouseWheel, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessTextInput(core::E_Character const character) override;
	// #todo: WindowFocusCallback
	// #todo: CursorEnterCallback

	// utility
	//---------
private:
	void UpdateMouseCursor();
	void UpdateKeyModifiers(core::T_KeyModifierFlags const modifiers) const;


	// Data
	///////

	Ptr<core::I_CursorShapeManager> m_CursorShapeManager;
	Ptr<core::I_ClipboardController> m_ClipboardController;

	Ptr<render::Viewport> m_Viewport;

	vec2 m_LastValidMousePos;

	std::string m_LastClipboardText;
};


} // namespace gui
} // namespace et


#endif // ndef IMGUI_DISABLE
