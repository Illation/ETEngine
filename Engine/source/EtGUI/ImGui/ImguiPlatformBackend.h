#pragma once
#include <imgui/imgui.h>

#if ET_CT_IS_ENABLED(ET_CT_IMGUI)

#include <EtCore/Input/RawInputListener.h>
#include <EtCore/Input/CursorShapes.h>
#include <EtCore/Input/ClipboardControllerInterface.h>

#include <EtRHI/GraphicsContext/GraphicsContext.h>
#include <EtRHI/GraphicsContext/Viewport.h>


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
		Ptr<rhi::Viewport> const viewport);
	void Deinit();

	// functionality
	//---------------
	void Update();

	// accessors
	//-----------
	rhi::Viewport* GetViewport() { return m_Viewport.Get(); }

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
	bool ProcessMouseEnterLeave(bool const entered, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessTextInput(core::E_Character const character) override;
	// #todo: WindowFocusCallback

	// utility
	//---------
private:
	void UpdateMouseCursor();
	void UpdateKeyModifiers(core::T_KeyModifierFlags const modifiers) const;


	// Data
	///////

	Ptr<core::I_CursorShapeManager> m_CursorShapeManager;
	core::E_CursorShape m_LastCursorShape;

	Ptr<core::I_ClipboardController> m_ClipboardController;
	std::string m_LastClipboardText;

	Ptr<rhi::Viewport> m_Viewport;

	vec2 m_LastValidMousePos;
};


} // namespace gui
} // namespace et


#endif // ET_CT_IS_ENABLED(ET_CT_IMGUI)
