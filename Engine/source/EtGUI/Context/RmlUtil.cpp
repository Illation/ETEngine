#include "stdafx.h"
#include "RmlUtil.h"



namespace et {
namespace gui {


//==========
// Rml Util
//==========


//---------------------------------
// RmlUtil::GetRmlButtonIndex
//
// Convert Engine input button codes to Rml mouse button indices
//
int32 RmlUtil::GetRmlButtonIndex(E_MouseButton const button)
{
	switch (button)
	{
	case E_MouseButton::Left: return 0;
	case E_MouseButton::Right: return 1;
	case E_MouseButton::Center: return 2;

	default: return -1;
	}
}

//---------------------------------
// RmlUtil::GetRmlKeyId
//
// Convert Engine input buttons to Rml key identifiers
//
Rml::Input::KeyIdentifier RmlUtil::GetRmlKeyId(E_KbdKey const key)
{
	switch (key)
	{
	case E_KbdKey::Space: return Rml::Input::KI_SPACE;
	case E_KbdKey::Apostrophe: return Rml::Input::KI_OEM_7;
	case E_KbdKey::Comma: return Rml::Input::KI_OEM_COMMA;
	case E_KbdKey::Minus: return Rml::Input::KI_OEM_MINUS;
	case E_KbdKey::Period: return Rml::Input::KI_OEM_PERIOD;
	case E_KbdKey::Slash: return Rml::Input::KI_OEM_2;

	case E_KbdKey::Num_0: return Rml::Input::KI_0;
	case E_KbdKey::Num_1: return Rml::Input::KI_1;
	case E_KbdKey::Num_2: return Rml::Input::KI_2;
	case E_KbdKey::Num_3: return Rml::Input::KI_3;
	case E_KbdKey::Num_4: return Rml::Input::KI_4;
	case E_KbdKey::Num_5: return Rml::Input::KI_5;
	case E_KbdKey::Num_6: return Rml::Input::KI_6;
	case E_KbdKey::Num_7: return Rml::Input::KI_7;
	case E_KbdKey::Num_8: return Rml::Input::KI_8;
	case E_KbdKey::Num_9: return Rml::Input::KI_9;

	case E_KbdKey::Semicolon: return Rml::Input::KI_OEM_1;
	case E_KbdKey::Equal: return Rml::Input::KI_OEM_PLUS;

	case E_KbdKey::A: return Rml::Input::KI_A;
	case E_KbdKey::B: return Rml::Input::KI_B;
	case E_KbdKey::C: return Rml::Input::KI_C;
	case E_KbdKey::D: return Rml::Input::KI_D;
	case E_KbdKey::E: return Rml::Input::KI_E;
	case E_KbdKey::F: return Rml::Input::KI_F;
	case E_KbdKey::G: return Rml::Input::KI_G;
	case E_KbdKey::H: return Rml::Input::KI_H;
	case E_KbdKey::I: return Rml::Input::KI_I;
	case E_KbdKey::J: return Rml::Input::KI_J;
	case E_KbdKey::K: return Rml::Input::KI_K;
	case E_KbdKey::L: return Rml::Input::KI_L;
	case E_KbdKey::M: return Rml::Input::KI_M;
	case E_KbdKey::N: return Rml::Input::KI_N;
	case E_KbdKey::O: return Rml::Input::KI_O;
	case E_KbdKey::P: return Rml::Input::KI_P;
	case E_KbdKey::Q: return Rml::Input::KI_Q;
	case E_KbdKey::R: return Rml::Input::KI_R;
	case E_KbdKey::S: return Rml::Input::KI_S;
	case E_KbdKey::T: return Rml::Input::KI_T;
	case E_KbdKey::U: return Rml::Input::KI_U;
	case E_KbdKey::V: return Rml::Input::KI_V;
	case E_KbdKey::W: return Rml::Input::KI_W;
	case E_KbdKey::X: return Rml::Input::KI_X;
	case E_KbdKey::Y: return Rml::Input::KI_Y;
	case E_KbdKey::Z: return Rml::Input::KI_Z;

	case E_KbdKey::LeftBracket: return Rml::Input::KI_OEM_4;
	case E_KbdKey::Backslash: return Rml::Input::KI_OEM_5;
	case E_KbdKey::RightBracket: return Rml::Input::KI_OEM_6;
	case E_KbdKey::GraveAccent: return Rml::Input::KI_OEM_7;
	//case E_KbdKey::World_1: return Rml::Input::KI_OEM_PERIOD;
	//case E_KbdKey::World_2: return Rml::Input::KI_OEM_2;

	case E_KbdKey::Escape: return Rml::Input::KI_ESCAPE;
	case E_KbdKey::Return: return Rml::Input::KI_RETURN;
	case E_KbdKey::Tab: return Rml::Input::KI_TAB;
	case E_KbdKey::Backspace: return Rml::Input::KI_BACK;
	case E_KbdKey::Insert: return Rml::Input::KI_INSERT;
	case E_KbdKey::Delete: return Rml::Input::KI_DELETE;
	case E_KbdKey::Right: return Rml::Input::KI_RIGHT;
	case E_KbdKey::Left: return Rml::Input::KI_LEFT;
	case E_KbdKey::Down: return Rml::Input::KI_DOWN;
	case E_KbdKey::Up: return Rml::Input::KI_UP;
	case E_KbdKey::PageUp: return Rml::Input::KI_PRIOR;
	case E_KbdKey::PageDown: return Rml::Input::KI_NEXT;
	case E_KbdKey::Home: return Rml::Input::KI_HOME;
	case E_KbdKey::End: return Rml::Input::KI_END;
	case E_KbdKey::CapsLock: return Rml::Input::KI_CAPITAL;
	case E_KbdKey::ScrollLock: return Rml::Input::KI_SCROLL;
	case E_KbdKey::NumLock: return Rml::Input::KI_NUMLOCK;
	case E_KbdKey::PrintScreen: return Rml::Input::KI_SNAPSHOT;
	case E_KbdKey::Pause: return Rml::Input::KI_PAUSE;

	case E_KbdKey::F1: return Rml::Input::KI_F1;
	case E_KbdKey::F2: return Rml::Input::KI_F2;
	case E_KbdKey::F3: return Rml::Input::KI_F3;
	case E_KbdKey::F4: return Rml::Input::KI_F4;
	case E_KbdKey::F5: return Rml::Input::KI_F5;
	case E_KbdKey::F6: return Rml::Input::KI_F6;
	case E_KbdKey::F7: return Rml::Input::KI_F7;
	case E_KbdKey::F8: return Rml::Input::KI_F8;
	case E_KbdKey::F9: return Rml::Input::KI_F9;
	case E_KbdKey::F10: return Rml::Input::KI_F10;
	case E_KbdKey::F11: return Rml::Input::KI_F11;
	case E_KbdKey::F12: return Rml::Input::KI_F12;
	case E_KbdKey::F13: return Rml::Input::KI_F13;
	case E_KbdKey::F14: return Rml::Input::KI_F14;
	case E_KbdKey::F15: return Rml::Input::KI_F15;
	case E_KbdKey::F16: return Rml::Input::KI_F16;
	case E_KbdKey::F17: return Rml::Input::KI_F17;
	case E_KbdKey::F18: return Rml::Input::KI_F18;
	case E_KbdKey::F19: return Rml::Input::KI_F19;
	case E_KbdKey::F20: return Rml::Input::KI_F20;
	case E_KbdKey::F21: return Rml::Input::KI_F21;
	case E_KbdKey::F22: return Rml::Input::KI_F22;
	case E_KbdKey::F23: return Rml::Input::KI_F23;
	case E_KbdKey::F24: return Rml::Input::KI_F24;

	case E_KbdKey::KP_0: return Rml::Input::KI_NUMPAD0;
	case E_KbdKey::KP_1: return Rml::Input::KI_NUMPAD1;
	case E_KbdKey::KP_2: return Rml::Input::KI_NUMPAD2;
	case E_KbdKey::KP_3: return Rml::Input::KI_NUMPAD3;
	case E_KbdKey::KP_4: return Rml::Input::KI_NUMPAD4;
	case E_KbdKey::KP_5: return Rml::Input::KI_NUMPAD5;
	case E_KbdKey::KP_6: return Rml::Input::KI_NUMPAD6;
	case E_KbdKey::KP_7: return Rml::Input::KI_NUMPAD7;
	case E_KbdKey::KP_8: return Rml::Input::KI_NUMPAD8;
	case E_KbdKey::KP_9: return Rml::Input::KI_NUMPAD9;

	case E_KbdKey::KP_Decimal: return Rml::Input::KI_DECIMAL;
	case E_KbdKey::KP_Divide: return Rml::Input::KI_DIVIDE;
	case E_KbdKey::KP_Mult: return Rml::Input::KI_MULTIPLY;
	case E_KbdKey::KP_Minus: return Rml::Input::KI_SUBTRACT;
	case E_KbdKey::KP_Plus: return Rml::Input::KI_ADD;
	case E_KbdKey::KP_Enter: return Rml::Input::KI_NUMPADENTER;
	case E_KbdKey::KP_Equal: return Rml::Input::KI_OEM_NEC_EQUAL;

	case E_KbdKey::LeftShift: return Rml::Input::KI_LSHIFT;
	case E_KbdKey::LeftControl: return Rml::Input::KI_LCONTROL;
	//case E_KbdKey::LeftAlt: return Rml::Input::KI_LMENU; // might be incorrect
	case E_KbdKey::LeftSuper: return Rml::Input::KI_LWIN;
	case E_KbdKey::RightShift: return Rml::Input::KI_RSHIFT;
	case E_KbdKey::RightControl: return Rml::Input::KI_RCONTROL;
	//case E_KbdKey::RightAlt: return Rml::Input::KI_RMENU; // might be incorrect
	case E_KbdKey::RightSuper: return Rml::Input::KI_RWIN;
	//case E_KbdKey::Menu: return Rml::Input::KI_APPS; // might be incorrect

	default:
		return Rml::Input::KI_UNKNOWN;
	}
}

//---------------------------------
// RmlUtil::GetRmlModifierFlags
//
// Convert Engine modifier flags to Rml modifier flags
//
int32 RmlUtil::GetRmlModifierFlags(core::T_KeyModifierFlags const mods)
{
	int ret = 0;

	if (mods & core::E_KeyModifier::KM_Shift)
	{
		ret |= Rml::Input::KeyModifier::KM_SHIFT;
	}

	if (mods & core::E_KeyModifier::KM_Control)
	{
		ret |= Rml::Input::KeyModifier::KM_CTRL;
	}

	if (mods & core::E_KeyModifier::KM_Alt)
	{
		ret |= Rml::Input::KeyModifier::KM_ALT;
	}

	if (mods & core::E_KeyModifier::KM_Super)
	{
		ret |= Rml::Input::KeyModifier::KM_META;
	}

	if (mods & core::E_KeyModifier::KM_CapsLock)
	{
		ret |= Rml::Input::KeyModifier::KM_CAPSLOCK;
	}

	if (mods & core::E_KeyModifier::KM_NumLock)
	{
		ret |= Rml::Input::KeyModifier::KM_NUMLOCK;
	}

	return ret;
}


} // namespace gui
} // namespace et


