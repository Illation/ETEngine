#pragma once

#include <iostream>

#include "TickOrder.h"
#include "GlfwRenderArea.h"

#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>
#include <EtCore/UpdateCycle/Tickable.h>


class Viewport;


class AbstractFramework : public I_RealTimeTickTriggerer, public I_Tickable
{
public:
	AbstractFramework() : I_Tickable(static_cast<uint32>(E_TickOrder::TICK_Framework)) {}
	virtual ~AbstractFramework();

	void Run();

protected:
	virtual void AddScenes() = 0;
	virtual void OnTick() override = 0;

private:
	void LoadConfig();
	void InitializeGame();

	void GameLoop();

private:
	GlfwRenderArea m_RenderArea;
	Viewport* m_Viewport = nullptr;
};
