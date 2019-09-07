#pragma once

#include <map>
#include <SDL.h>

#include <EtCore/UpdateCycle/Tickable.h>
#include <EtCore/Helper/Singleton.h>

#include <Engine/Base/TickOrder.h>


//----------------------------
// SdlEventManager
//
// Responsible for funelling SDL events into the input manager
//
class SdlEventManager : public Singleton<SdlEventManager>, public I_Tickable, public I_CursorShapeManager
{
private:
	// definitions
	//--------------
	friend class AbstractFramework;
public:
	// ctor dtor
	//---------------
	SdlEventManager() : I_Tickable(static_cast<uint32>(E_TickOrder::TICK_SdlEventManager)) {}
	virtual ~SdlEventManager();

	// modify state
	//--------------
protected:
	bool OnCursorResize(E_CursorShape const shape) override;
private:
	void Init(); // call after SDL init
	void OnTick() override; // call before all GUI ticks

	// utility
	//--------------
	E_MouseButton GetButtonFromSdl(SDL_Event &evnt);
	static E_KbdKey ConvertSdlKeyCode(SDL_Keycode const code);

	// Data 
	/////////

	// Cursors
	std::map<E_CursorShape, SDL_Cursor*> m_CursorMap;
};

