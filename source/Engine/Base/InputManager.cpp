#include "stdafx.hpp"
#include "InputManager.hpp"

//----------------------------
//Constructor and Destructor
//----------------------------
InputManager::InputManager()
{
	m_CharToSdlMap['A'] = SDL_SCANCODE_A;
	m_CharToSdlMap['B'] = SDL_SCANCODE_B;
	m_CharToSdlMap['C'] = SDL_SCANCODE_C;
	m_CharToSdlMap['D'] = SDL_SCANCODE_D;
	m_CharToSdlMap['E'] = SDL_SCANCODE_E;
	m_CharToSdlMap['F'] = SDL_SCANCODE_F;
	m_CharToSdlMap['G'] = SDL_SCANCODE_G;
	m_CharToSdlMap['H'] = SDL_SCANCODE_H;
	m_CharToSdlMap['I'] = SDL_SCANCODE_I;
	m_CharToSdlMap['J'] = SDL_SCANCODE_J;
	m_CharToSdlMap['K'] = SDL_SCANCODE_K;
	m_CharToSdlMap['L'] = SDL_SCANCODE_L;
	m_CharToSdlMap['M'] = SDL_SCANCODE_M;
	m_CharToSdlMap['N'] = SDL_SCANCODE_N;
	m_CharToSdlMap['O'] = SDL_SCANCODE_O;
	m_CharToSdlMap['P'] = SDL_SCANCODE_P;
	m_CharToSdlMap['Q'] = SDL_SCANCODE_Q;
	m_CharToSdlMap['R'] = SDL_SCANCODE_R;
	m_CharToSdlMap['S'] = SDL_SCANCODE_S;
	m_CharToSdlMap['T'] = SDL_SCANCODE_T;
	m_CharToSdlMap['U'] = SDL_SCANCODE_U;
	m_CharToSdlMap['V'] = SDL_SCANCODE_V;
	m_CharToSdlMap['W'] = SDL_SCANCODE_W;
	m_CharToSdlMap['X'] = SDL_SCANCODE_X;
	m_CharToSdlMap['Y'] = SDL_SCANCODE_Y;
	m_CharToSdlMap['Z'] = SDL_SCANCODE_Z;

	m_CharToSdlMap['0'] = SDL_SCANCODE_0;
	m_CharToSdlMap['1'] = SDL_SCANCODE_1;
	m_CharToSdlMap['2'] = SDL_SCANCODE_2;
	m_CharToSdlMap['3'] = SDL_SCANCODE_3;
	m_CharToSdlMap['4'] = SDL_SCANCODE_4;
	m_CharToSdlMap['5'] = SDL_SCANCODE_5;
	m_CharToSdlMap['6'] = SDL_SCANCODE_6;
	m_CharToSdlMap['7'] = SDL_SCANCODE_7;
	m_CharToSdlMap['8'] = SDL_SCANCODE_8;
	m_CharToSdlMap['9'] = SDL_SCANCODE_9;
}
InputManager::~InputManager()
{
	delete m_KeyMapNew;
	m_KeyMapNew = nullptr;
	delete m_KeyMapOld;
	m_KeyMapOld = nullptr;
	if (!(m_KeyMapSdl[0] == '\0'))
	{
		delete m_KeyMapSdl;
		m_KeyMapSdl = nullptr;
	}

	for (auto it = m_CursorMap.begin(); it != m_CursorMap.end(); ++it)
	{
		SDL_FreeCursor(it->second);
	}
}

//----------------------------
//MemberFunctions
//----------------------------
void InputManager::Init()
{
	m_KeyMapSdl = SDL_GetKeyboardState(&m_KeyboardLength);//Set this map to be constantly updated
	m_KeyMapNew = new uint8[m_KeyboardLength];
	m_KeyMapOld = new uint8[m_KeyboardLength];
	m_MouseMapNew = SDL_GetMouseState(NULL, NULL);
	m_MouseMapOld = m_MouseMapNew;

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
void InputManager::UpdateEvents()
{
	std::memcpy(m_KeyMapOld, m_KeyMapNew, m_KeyboardLength);//Update Old Keyboard state
	m_MouseMapOld = m_MouseMapNew;//Update old Mouse state
	//Pump SDL events
	SDL_Event evnt;
	bool mouseWheel = false;
	while (SDL_PollEvent(&evnt))
	{
		switch (evnt.type)
		{
		case SDL_QUIT:
			m_ExitRequested = true;
			break;
		case SDL_WINDOWEVENT:
			switch (evnt.window.event)
			{
			case SDL_WINDOWEVENT_CLOSE:
				m_ExitRequested = true;
				break;
			case SDL_WINDOWEVENT_RESIZED:
				WINDOW.Resize( evnt.window.data1, evnt.window.data2 );
				break;
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				WINDOW.Resize( evnt.window.data1, evnt.window.data2 );
				break;
			}
			break;
		case SDL_MOUSEWHEEL:
			mouseWheel = true;
			m_MouseWheelDelta.x = (float)evnt.wheel.x;
			m_MouseWheelDelta.y = (float)evnt.wheel.y;
			break;
		}
	}
	if (!mouseWheel)m_MouseWheelDelta = vec2();
	std::memcpy(m_KeyMapNew, m_KeyMapSdl, m_KeyboardLength);//Update New Keyboard state
	int32 mPosOldX = m_MousePosX;
	int32 mPosOldY = m_MousePosY;
	m_MouseMapNew = SDL_GetMouseState(&m_MousePosX, &m_MousePosY);//Update new Mouse state and position
	m_MouseMove = vec2((float)(m_MousePosX - mPosOldX), (float)(m_MousePosY- mPosOldY));//maybe divide by screen resolution

	m_MouseConsumed = false;
}
//----------------------------
//Getters
//----------------------------
bool InputManager::IsExitRequested()
{
	return m_ExitRequested;
}
//----------------------------
//Keyboard
//----------------------------
bool InputManager::GetScancode(SDL_Scancode &code, char key)
{
	if (m_CharToSdlMap.find(key) == m_CharToSdlMap.end()) {
		return false;
	}
	else {
		code = m_CharToSdlMap[key];
		return true;
	}
}
bool InputManager::IsKeyboardKeyPressed(SDL_Scancode key)
{
	return m_KeyMapNew[key] && !m_KeyMapOld[key];
}
bool InputManager::IsKeyboardKeyPressed(char key)
{
	SDL_Scancode sKey;
	if (GetScancode(sKey, key))
	{
		return m_KeyMapNew[sKey] && !m_KeyMapOld[sKey];
	}
	return false;
}
bool InputManager::IsKeyboardKeyDown(SDL_Scancode key)
{
	return m_KeyMapNew[key] && m_KeyMapOld[key];
}
bool InputManager::IsKeyboardKeyDown(char key)
{
	SDL_Scancode sKey;
	if (GetScancode(sKey, key))
	{
		return m_KeyMapNew[sKey] && m_KeyMapOld[sKey];
	}
	return false;
}
bool InputManager::IsKeyboardKeyReleased(SDL_Scancode key)
{
	return !m_KeyMapNew[key] && m_KeyMapOld[key];
}
bool InputManager::IsKeyboardKeyReleased(char key)
{
	SDL_Scancode sKey;
	if (GetScancode(sKey, key))
	{
		return !m_KeyMapNew[sKey] && m_KeyMapOld[sKey];
	}
	return false;
}
//----------------------------
//Mouse
//----------------------------
bool InputManager::IsMouseButtonPressed(int32 button)
{
	if (m_MouseConsumed)return false;
	return (m_MouseMapNew & SDL_BUTTON(button)) && 
		!(m_MouseMapOld & SDL_BUTTON(button));
}
bool InputManager::IsMouseButtonDown(int32 button)
{
	if (m_MouseConsumed)return false;
	return (m_MouseMapNew & SDL_BUTTON(button)) &&
		(m_MouseMapOld & SDL_BUTTON(button));
}
bool InputManager::IsMouseButtonReleased(int32 button)
{
	if (m_MouseConsumed)return false;
	return !(m_MouseMapNew & SDL_BUTTON(button)) &&
		(m_MouseMapOld & SDL_BUTTON(button));
}
vec2 InputManager::GetMousePosition()
{
	return vec2((float)m_MousePosX, (float)m_MousePosY);
}

void InputManager::SetSystemCursor(SDL_SystemCursor cursor)
{
	auto it = m_CursorMap.find(cursor);
	if(it != m_CursorMap.end())
	{
		SDL_SetCursor(it->second);
	}
	else
	{
		std::cout << "[WARNING] INPUT::SetSystemCursor > cursor not found" << std::endl;
	}
}

void InputManager::ConsumeMouse()
{
	m_MouseConsumed = true;
}