#include "stdafx.h"
#include "SdlEventManager.h"

#include <EtCore/Helper/InputManager.h>


//======================
// SDL Event Manager
//=====================


//----------------------------
// SdlEventManager::SdlEventManager
//
// SDL event manager dtor
//
SdlEventManager::~SdlEventManager()
{
	for (auto it = m_CursorMap.begin(); it != m_CursorMap.end(); ++it)
	{
		SDL_FreeCursor(it->second);
	}
}

//----------------------------
// SdlEventManager::OnCursorResize
//
// Change the cursor currently used
//
bool SdlEventManager::OnCursorResize(E_CursorShape const shape)
{
	auto it = m_CursorMap.find(shape);
	if (it != m_CursorMap.end())
	{
		SDL_SetCursor(it->second);
		return true;
	}

	LOG("INPUT::OnCursorResize > Shape not found", Warning);
	return false;
}

//----------------------------
// SdlEventManager::Init
//
// Init all possible cursors, and register as a cursor manager with the input manager
//
void SdlEventManager::Init()
{
	m_CursorMap[E_CursorShape::Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	m_CursorMap[E_CursorShape::IBeam] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	m_CursorMap[E_CursorShape::Crosshair] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	m_CursorMap[E_CursorShape::SizeWE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	m_CursorMap[E_CursorShape::SizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	m_CursorMap[E_CursorShape::Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);

	InputManager::GetInstance()->RegisterCursorShapeManager(this);
}

//----------------------------
// SdlEventManager::OnTick
//
// Pump the SDL message loop into the input manager
//
void SdlEventManager::OnTick()
{
	//Pump SDL events
	SDL_Event evnt;
	while (SDL_PollEvent(&evnt))
	{
		switch (evnt.type)
		{
		case SDL_QUIT:
			InputManager::GetInstance()->Quit();
			break;
		case SDL_WINDOWEVENT:
			switch (evnt.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				InputManager::GetInstance()->Quit();
				break;
			case SDL_WINDOWEVENT_RESIZED:
				WINDOW.Resize( evnt.window.data1, evnt.window.data2 );
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				WINDOW.Resize( evnt.window.data1, evnt.window.data2 );
				break;
			}
			break;

		case SDL_KEYDOWN:
			InputManager::GetInstance()->OnKeyPressed(ConvertSdlKeyCode(evnt.key.keysym.sym));
			break;
		case SDL_KEYUP:
			InputManager::GetInstance()->OnKeyReleased(ConvertSdlKeyCode(evnt.key.keysym.sym));
			break;
		case SDL_MOUSEMOTION:
			InputManager::GetInstance()->OnMouseMoved(ivec2(evnt.motion.x, evnt.motion.y));
			break;
		case SDL_MOUSEBUTTONDOWN:
			{
				E_MouseButton button = GetButtonFromSdl(evnt);
				if (button != E_MouseButton::COUNT)
				{
					InputManager::GetInstance()->OnMousePressed(button);
				}
			}
			break;
		case SDL_MOUSEBUTTONUP:
			{
				E_MouseButton button = GetButtonFromSdl(evnt);
				if (button != E_MouseButton::COUNT)
				{
					InputManager::GetInstance()->OnMouseReleased(button);
				}
			}
			break;
		case SDL_MOUSEWHEEL:
			InputManager::GetInstance()->SetMouseWheelDelta(ivec2(evnt.wheel.x, evnt.wheel.y));
			break;
		}
	}
}

//----------------------------
// SdlEventManager::UpdateEvents
//
// Convert the events button to an E_MouseButton for the input manager
//
E_MouseButton SdlEventManager::GetButtonFromSdl(SDL_Event &evnt)
{
	switch (evnt.button.button)
	{
	case SDL_BUTTON(SDL_BUTTON_LEFT):
		return E_MouseButton::Left;
	case SDL_BUTTON(SDL_BUTTON_MIDDLE):
		return E_MouseButton::Center;
	case SDL_BUTTON(SDL_BUTTON_RIGHT):
		return E_MouseButton::Right;
	default:
		return E_MouseButton::COUNT;
	}			
}

//----------------------------
// SdlEventManager::ConvertSdlKeyCode
//
// Convert an SDL_Keycode to the engines library agnostic format
//
E_KbdKey SdlEventManager::ConvertSdlKeyCode(SDL_Keycode const code)
{
	switch (code)
	{
	case SDLK_SPACE: return E_KbdKey::Space;
	case SDLK_QUOTE: return E_KbdKey::Apostrophe;
	case SDLK_COMMA: return E_KbdKey::Comma;
	case SDLK_MINUS: return E_KbdKey::Minus;
	case SDLK_PERIOD: return E_KbdKey::Period;
	case SDLK_SLASH: return E_KbdKey::Slash;

	case SDLK_0: return E_KbdKey::Num_0;
	case SDLK_1: return E_KbdKey::Num_1;
	case SDLK_2: return E_KbdKey::Num_2;
	case SDLK_3: return E_KbdKey::Num_3;
	case SDLK_4: return E_KbdKey::Num_4;
	case SDLK_5: return E_KbdKey::Num_5;
	case SDLK_6: return E_KbdKey::Num_6;
	case SDLK_7: return E_KbdKey::Num_7;
	case SDLK_8: return E_KbdKey::Num_8;
	case SDLK_9: return E_KbdKey::Num_9;

	case SDLK_SEMICOLON: return E_KbdKey::Semicolon;
	case SDLK_EQUALS: return E_KbdKey::Equal;

	case SDLK_a: return E_KbdKey::A;
	case SDLK_b: return E_KbdKey::B;
	case SDLK_c: return E_KbdKey::C;
	case SDLK_d: return E_KbdKey::D;
	case SDLK_e: return E_KbdKey::E;
	case SDLK_f: return E_KbdKey::F;
	case SDLK_g: return E_KbdKey::G;
	case SDLK_h: return E_KbdKey::H;
	case SDLK_i: return E_KbdKey::I;
	case SDLK_j: return E_KbdKey::J;
	case SDLK_k: return E_KbdKey::K;
	case SDLK_l: return E_KbdKey::L;
	case SDLK_m: return E_KbdKey::M;
	case SDLK_n: return E_KbdKey::N;
	case SDLK_o: return E_KbdKey::O;
	case SDLK_p: return E_KbdKey::P;
	case SDLK_q: return E_KbdKey::Q;
	case SDLK_r: return E_KbdKey::R;
	case SDLK_s: return E_KbdKey::S;
	case SDLK_t: return E_KbdKey::T;
	case SDLK_u: return E_KbdKey::U;
	case SDLK_v: return E_KbdKey::V;
	case SDLK_w: return E_KbdKey::W;
	case SDLK_x: return E_KbdKey::X;
	case SDLK_y: return E_KbdKey::Y;
	case SDLK_z: return E_KbdKey::Z;

	case SDLK_LEFTBRACKET: return E_KbdKey::LeftBracket;
	case SDLK_BACKSLASH: return E_KbdKey::Backslash;
	case SDLK_RIGHTBRACKET: return E_KbdKey::RightBracket;
	case SDLK_BACKQUOTE: return E_KbdKey::GraveAccent;
	//case SDLK_RETURN: return E_KbdKey::World_1;
	//case SDLK_RETURN: return E_KbdKey::World_2;

	case SDLK_ESCAPE: return E_KbdKey::Escape;
	case SDLK_RETURN: return E_KbdKey::Return;
	case SDLK_TAB: return E_KbdKey::Tab;
	case SDLK_BACKSPACE: return E_KbdKey::Backspace;
	case SDLK_INSERT: return E_KbdKey::Insert;
	case SDLK_DELETE: return E_KbdKey::Delete;
	case SDLK_RIGHT: return E_KbdKey::Right;
	case SDLK_LEFT: return E_KbdKey::Left;
	case SDLK_DOWN: return E_KbdKey::Down;
	case SDLK_UP: return E_KbdKey::Up;
	case SDLK_PAGEUP: return E_KbdKey::PageUp;
	case SDLK_PAGEDOWN: return E_KbdKey::PageDown;
	case SDLK_HOME: return E_KbdKey::Home;
	case SDLK_END: return E_KbdKey::End;
	case SDLK_CAPSLOCK: return E_KbdKey::CapsLock;
	case SDLK_SCROLLLOCK: return E_KbdKey::ScrollLock;
	case SDLK_NUMLOCKCLEAR: return E_KbdKey::NumLock;
	case SDLK_PRINTSCREEN: return E_KbdKey::PrintScreen;
	case SDLK_PAUSE: return E_KbdKey::Pause;

	case SDLK_F1: return E_KbdKey::F1;
	case SDLK_F2: return E_KbdKey::F2;
	case SDLK_F3: return E_KbdKey::F3;
	case SDLK_F4: return E_KbdKey::F4;
	case SDLK_F5: return E_KbdKey::F5;
	case SDLK_F6: return E_KbdKey::F6;
	case SDLK_F7: return E_KbdKey::F7;
	case SDLK_F8: return E_KbdKey::F8;
	case SDLK_F9: return E_KbdKey::F9;
	case SDLK_F10: return E_KbdKey::F10;
	case SDLK_F11: return E_KbdKey::F11;
	case SDLK_F12: return E_KbdKey::F12;
	case SDLK_F13: return E_KbdKey::F13;
	case SDLK_F14: return E_KbdKey::F14;
	case SDLK_F15: return E_KbdKey::F15;
	case SDLK_F16: return E_KbdKey::F16;
	case SDLK_F17: return E_KbdKey::F17;
	case SDLK_F18: return E_KbdKey::F18;
	case SDLK_F19: return E_KbdKey::F19;
	case SDLK_F20: return E_KbdKey::F20;
	case SDLK_F21: return E_KbdKey::F21;
	case SDLK_F22: return E_KbdKey::F22;
	case SDLK_F23: return E_KbdKey::F23;
	case SDLK_F24: return E_KbdKey::F24;
	//case SDLK_RETURN: return E_KbdKey::F25;

	case SDLK_KP_0: return E_KbdKey::KP_0;
	case SDLK_KP_1: return E_KbdKey::KP_1;
	case SDLK_KP_2: return E_KbdKey::KP_2;
	case SDLK_KP_3: return E_KbdKey::KP_3;
	case SDLK_KP_4: return E_KbdKey::KP_4;
	case SDLK_KP_5: return E_KbdKey::KP_5;
	case SDLK_KP_6: return E_KbdKey::KP_6;
	case SDLK_KP_7: return E_KbdKey::KP_7;
	case SDLK_KP_8: return E_KbdKey::KP_8;
	case SDLK_KP_9: return E_KbdKey::KP_9;

	case SDLK_KP_DECIMAL: return E_KbdKey::KP_Decimal;
	case SDLK_KP_DIVIDE: return E_KbdKey::KP_Divide;
	case SDLK_KP_MULTIPLY: return E_KbdKey::KP_Mult;
	case SDLK_KP_MINUS: return E_KbdKey::KP_Minus;
	case SDLK_KP_PLUS: return E_KbdKey::KP_Plus;
	case SDLK_KP_ENTER: return E_KbdKey::KP_Enter;
	case SDLK_KP_EQUALS: return E_KbdKey::KP_Equal;

	case SDLK_LSHIFT: return E_KbdKey::LeftShift;
	case SDLK_LCTRL: return E_KbdKey::LeftControl;
	case SDLK_LALT: return E_KbdKey::LeftAlt;
	case SDLK_LGUI: return E_KbdKey::LeftSuper;
	case SDLK_RSHIFT: return E_KbdKey::RightShift;
	case SDLK_RCTRL: return E_KbdKey::RightControl;
	case SDLK_RALT: return E_KbdKey::RightAlt;
	case SDLK_RGUI: return E_KbdKey::RightSuper;
	case SDLK_MENU: return E_KbdKey::Menu;

	default: return E_KbdKey::Invalid; 
	}								   
}									   