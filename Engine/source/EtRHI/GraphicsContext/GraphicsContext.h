#pragma once
#include "RenderWindow.h"


namespace et {
namespace rhi {


//---------------------------------
// GraphicsContext
//
// Holds all the data related to a rendering context
//
struct GraphicsContext
{
	// construct
	//-----------
	GraphicsContext() = default;
	GraphicsContext(Ptr<RenderWindow> const window, Ptr<I_RenderDevice> const device) : m_SourceWindow(window), m_Device(device) {}

	// accessors
	//-----------
	RenderWindow* GetSourceWindow() const { return m_SourceWindow.Get(); }
	I_RenderDevice* GetRenderDevice() const { return m_Device.Get(); }

	// Data
	///////
private:
	Ptr<RenderWindow> m_SourceWindow;
	Ptr<I_RenderDevice> m_Device;
};


} // namespace rhi
} // namespace et

