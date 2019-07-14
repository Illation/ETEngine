#pragma once

#include <map>
#include <SDL.h>

#include <Engine/Helper/Singleton.h>


//----------------------------
// SdlEventManager
//
// Responsible for funelling SDL events into the input manager
//
class SdlEventManager : public Singleton<SdlEventManager>
{
private:
	// definitions
	//--------------
	friend class AbstractFramework;
public:
	// ctor dtor
	//---------------
	SdlEventManager() = default;
	virtual ~SdlEventManager();

	// modify state
	//--------------
	void SetSystemCursor(SDL_SystemCursor cursor);
private:
	void Init(); // call after SDL init
	void UpdateEvents(); // call before all GUI ticks

	// utility
	//--------------
	E_MouseButton GetButtonFromSdl(SDL_Event &evnt);

	// Data 
	/////////

	// Cursors
	std::map<SDL_SystemCursor, SDL_Cursor*> m_CursorMap;
};

