#include "stdafx.h"
#include "ImGuiUtil.h"


#if ET_IMGUI_ENABLED


namespace et {
namespace gui {

namespace ImguiUtil {


//-------------
// GetButton
//
ImGuiMouseButton GetButton(E_MouseButton const button)
{
	switch (button)
	{
	case E_MouseButton::Left: return ImGuiMouseButton_Left;
	case E_MouseButton::Right: return ImGuiMouseButton_Right;
	case E_MouseButton::Center: return ImGuiMouseButton_Middle;

	default: return ImGuiMouseButton_COUNT;
	}
}

//-------------
// GetKey
//
ImGuiKey GetKey(E_KbdKey const key)
{
	switch (key)
	{
	case E_KbdKey::Tab: return ImGuiKey_Tab;
	case E_KbdKey::Left: return ImGuiKey_LeftArrow;
	case E_KbdKey::Right: return ImGuiKey_RightArrow;
	case E_KbdKey::Up: return ImGuiKey_UpArrow;
	case E_KbdKey::Down: return ImGuiKey_DownArrow;
	case E_KbdKey::PageUp: return ImGuiKey_PageUp;
	case E_KbdKey::PageDown: return ImGuiKey_PageDown;
	case E_KbdKey::Home: return ImGuiKey_Home;
	case E_KbdKey::End: return ImGuiKey_End;
	case E_KbdKey::Insert: return ImGuiKey_Insert;
	case E_KbdKey::Delete: return ImGuiKey_Delete;
	case E_KbdKey::Backspace: return ImGuiKey_Backspace;
	case E_KbdKey::Space: return ImGuiKey_Space;
	case E_KbdKey::Return: return ImGuiKey_Enter;
	case E_KbdKey::Escape: return ImGuiKey_Escape;
	case E_KbdKey::Apostrophe: return ImGuiKey_Apostrophe;
	case E_KbdKey::Comma: return ImGuiKey_Comma;
	case E_KbdKey::Minus: return ImGuiKey_Minus;
	case E_KbdKey::Period: return ImGuiKey_Period;
	case E_KbdKey::Slash: return ImGuiKey_Slash;
	case E_KbdKey::Semicolon: return ImGuiKey_Semicolon;
	case E_KbdKey::Equal: return ImGuiKey_Equal;
	case E_KbdKey::LeftBracket: return ImGuiKey_LeftBracket;
	case E_KbdKey::Backslash: return ImGuiKey_Backslash;
	case E_KbdKey::RightBracket: return ImGuiKey_RightBracket;
	case E_KbdKey::GraveAccent: return ImGuiKey_GraveAccent;
	case E_KbdKey::CapsLock: return ImGuiKey_CapsLock;
	case E_KbdKey::ScrollLock: return ImGuiKey_ScrollLock;
	case E_KbdKey::NumLock: return ImGuiKey_NumLock;
	case E_KbdKey::PrintScreen: return ImGuiKey_PrintScreen;
	case E_KbdKey::Pause: return ImGuiKey_Pause;

	case E_KbdKey::KP_0: return ImGuiKey_Keypad0;
	case E_KbdKey::KP_1: return ImGuiKey_Keypad1;
	case E_KbdKey::KP_2: return ImGuiKey_Keypad2;
	case E_KbdKey::KP_3: return ImGuiKey_Keypad3;
	case E_KbdKey::KP_4: return ImGuiKey_Keypad4;
	case E_KbdKey::KP_5: return ImGuiKey_Keypad5;
	case E_KbdKey::KP_6: return ImGuiKey_Keypad6;
	case E_KbdKey::KP_7: return ImGuiKey_Keypad7;
	case E_KbdKey::KP_8: return ImGuiKey_Keypad8;
	case E_KbdKey::KP_9: return ImGuiKey_Keypad9;

	case E_KbdKey::KP_Decimal: return ImGuiKey_KeypadDecimal;
	case E_KbdKey::KP_Divide: return ImGuiKey_KeypadDivide;
	case E_KbdKey::KP_Mult: return ImGuiKey_KeypadMultiply;
	case E_KbdKey::KP_Minus: return ImGuiKey_KeypadSubtract;
	case E_KbdKey::KP_Plus: return ImGuiKey_KeypadAdd;
	case E_KbdKey::KP_Enter: return ImGuiKey_KeypadEnter;
	case E_KbdKey::KP_Equal: return ImGuiKey_KeypadEqual;

	case E_KbdKey::LeftShift: return ImGuiKey_LeftShift;
	case E_KbdKey::LeftControl: return ImGuiKey_LeftCtrl;
	case E_KbdKey::LeftAlt: return ImGuiKey_LeftAlt;
	case E_KbdKey::LeftSuper: return ImGuiKey_LeftSuper;
	case E_KbdKey::RightShift: return ImGuiKey_RightShift;
	case E_KbdKey::RightControl: return ImGuiKey_RightCtrl;
	case E_KbdKey::RightAlt: return ImGuiKey_RightAlt;
	case E_KbdKey::RightSuper: return ImGuiKey_RightSuper;

	case E_KbdKey::Menu: return ImGuiKey_Menu;

	case E_KbdKey::Num_0: return ImGuiKey_0;
	case E_KbdKey::Num_1: return ImGuiKey_1;
	case E_KbdKey::Num_2: return ImGuiKey_2;
	case E_KbdKey::Num_3: return ImGuiKey_3;
	case E_KbdKey::Num_4: return ImGuiKey_4;
	case E_KbdKey::Num_5: return ImGuiKey_5;
	case E_KbdKey::Num_6: return ImGuiKey_6;
	case E_KbdKey::Num_7: return ImGuiKey_7;
	case E_KbdKey::Num_8: return ImGuiKey_8;
	case E_KbdKey::Num_9: return ImGuiKey_9;

	case E_KbdKey::A: return ImGuiKey_A;
	case E_KbdKey::B: return ImGuiKey_B;
	case E_KbdKey::C: return ImGuiKey_C;
	case E_KbdKey::D: return ImGuiKey_D;
	case E_KbdKey::E: return ImGuiKey_E;
	case E_KbdKey::F: return ImGuiKey_F;
	case E_KbdKey::G: return ImGuiKey_G;
	case E_KbdKey::H: return ImGuiKey_H;
	case E_KbdKey::I: return ImGuiKey_I;
	case E_KbdKey::J: return ImGuiKey_J;
	case E_KbdKey::K: return ImGuiKey_K;
	case E_KbdKey::L: return ImGuiKey_L;
	case E_KbdKey::M: return ImGuiKey_M;
	case E_KbdKey::N: return ImGuiKey_N;
	case E_KbdKey::O: return ImGuiKey_O;
	case E_KbdKey::P: return ImGuiKey_P;
	case E_KbdKey::Q: return ImGuiKey_Q;
	case E_KbdKey::R: return ImGuiKey_R;
	case E_KbdKey::S: return ImGuiKey_S;
	case E_KbdKey::T: return ImGuiKey_T;
	case E_KbdKey::U: return ImGuiKey_U;
	case E_KbdKey::V: return ImGuiKey_V;
	case E_KbdKey::W: return ImGuiKey_W;
	case E_KbdKey::X: return ImGuiKey_X;
	case E_KbdKey::Y: return ImGuiKey_Y;
	case E_KbdKey::Z: return ImGuiKey_Z;

	case E_KbdKey::F1: return ImGuiKey_F1;
	case E_KbdKey::F2: return ImGuiKey_F2;
	case E_KbdKey::F3: return ImGuiKey_F3;
	case E_KbdKey::F4: return ImGuiKey_F4;
	case E_KbdKey::F5: return ImGuiKey_F5;
	case E_KbdKey::F6: return ImGuiKey_F6;
	case E_KbdKey::F7: return ImGuiKey_F7;
	case E_KbdKey::F8: return ImGuiKey_F8;
	case E_KbdKey::F9: return ImGuiKey_F9;
	case E_KbdKey::F10: return ImGuiKey_F10;
	case E_KbdKey::F11: return ImGuiKey_F11;
	case E_KbdKey::F12: return ImGuiKey_F12;

	default: return ImGuiKey_None;
	}
}

//----------------
// GetCursorShape
//
core::E_CursorShape GetCursorShape(ImGuiMouseCursor const cursor)
{
	switch (cursor)
	{
	case ImGuiMouseCursor_None: return core::E_CursorShape::None;

	case ImGuiMouseCursor_TextInput: return core::E_CursorShape::IBeam;

		// #todo: would be nice to get better cursor shapes for these
	case ImGuiMouseCursor_ResizeAll:
	case ImGuiMouseCursor_ResizeNESW:
	case ImGuiMouseCursor_ResizeNWSE:
	case ImGuiMouseCursor_NotAllowed:
		return core::E_CursorShape::Crosshair;

	case ImGuiMouseCursor_Hand: return core::E_CursorShape::Hand;

	case ImGuiMouseCursor_ResizeEW: return core::E_CursorShape::SizeWE;

	case ImGuiMouseCursor_ResizeNS: return core::E_CursorShape::SizeNS;

	case ImGuiMouseCursor_Arrow:
	default:
		return core::E_CursorShape::Arrow;
	}
}


} // namespace ImguiUtil

} // namespace gui
} // namespace et


#endif // ET_IMGUI_ENABLED
