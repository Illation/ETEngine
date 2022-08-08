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
	GraphicsContext(RenderWindow* const window, I_GraphicsContextApi* const context) : m_SourceWindow(window), m_Context(context) {}

	// accessors
	//-----------
	RenderWindow* GetSourceWindow() const { return m_SourceWindow; }
	I_GraphicsContextApi* GetContext() const { return m_Context; }

	// Data
	///////
private:
	RenderWindow* m_SourceWindow = nullptr;
	I_GraphicsContextApi* m_Context = nullptr;
};


} // namespace rhi
} // namespace et

