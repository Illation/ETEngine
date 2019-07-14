#include "stdafx.h"
#include "InputManager.h"


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
// SdlEventManager::SetSystemCursor
//
// Change the cursor currently used
//
void SdlEventManager::SetSystemCursor(SDL_SystemCursor cursor)
{
	auto it = m_CursorMap.find(cursor);
	if (it != m_CursorMap.end())
	{
		SDL_SetCursor(it->second);
	}
	else
	{
		LOG("INPUT::SetSystemCursor > cursor not found", Warning);
	}
}

//----------------------------
// SdlEventManager::Init
//
// Init all possible cursors
//
void SdlEventManager::Init()
{
	m_CursorMap[SDL_SYSTEM_CURSOR_ARROW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
	m_CursorMap[SDL_SYSTEM_CURSOR_IBEAM] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
	m_CursorMap[SDL_SYSTEM_CURSOR_WAIT] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
	m_CursorMap[SDL_SYSTEM_CURSOR_CROSSHAIR] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
	m_CursorMap[SDL_SYSTEM_CURSOR_WAITARROW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAITARROW);
	m_CursorMap[SDL_SYSTEM_CURSOR_SIZENWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
	m_CursorMap[SDL_SYSTEM_CURSOR_SIZENESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
	m_CursorMap[SDL_SYSTEM_CURSOR_SIZEWE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
	m_CursorMap[SDL_SYSTEM_CURSOR_SIZENS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
	m_CursorMap[SDL_SYSTEM_CURSOR_SIZEALL] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
	m_CursorMap[SDL_SYSTEM_CURSOR_NO] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);
	m_CursorMap[SDL_SYSTEM_CURSOR_HAND] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
}

//----------------------------
// SdlEventManager::UpdateEvents
//
// Pump the SDL message loop into the input manager
//
void SdlEventManager::UpdateEvents()
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
			InputManager::GetInstance()->OnKeyPressed(static_cast<uint32>(evnt.key.keysym.sym));
			break;
		case SDL_KEYUP:
			InputManager::GetInstance()->OnKeyReleased(static_cast<uint32>(evnt.key.keysym.sym));
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
