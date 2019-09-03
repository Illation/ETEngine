#pragma once

#include <SDL.h>
#include <SDL_opengl.h>
#include <iostream>

#include "TickOrder.h"

#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>
#include <EtCore/UpdateCycle/Tickable.h>


//Static stuff
static void quit_SDL_error(const char * message);

class AbstractFramework : public I_RealTimeTickTriggerer, public I_Tickable
{
public:
	AbstractFramework() : I_Tickable(static_cast<uint32>(E_TickOrder::TICK_Framework)) {}
	virtual ~AbstractFramework();

	void Run();

protected:
	virtual void AddScenes() = 0;
	virtual void OnTick() override = 0;
	void ClearTarget();

private:
	void InitializeSDL();
	void LoadConfig();
	void InitializeWindow();
	void BindOpenGL();
	void InitializeDebug();
	void InitializeGame();

	void GameLoop();

private:
	SDL_GLContext m_GlContext = nullptr;
};

