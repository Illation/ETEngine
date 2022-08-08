#pragma once
#include "GraphicsContext.h"


namespace et {
namespace rhi {


//---------------------------------
// ContextHolder
//
// Container that holds all created graphics contexts and provides easy global access to them
//
class ContextHolder final : public RenderWindow::I_Listener
{
public:
	// static access
	//---------------
	static ContextHolder& Instance();
	static I_GraphicsContextApi* GetRenderContext();

	// accessors
	//-----------
	GraphicsContext& GetMainRenderContext() { return m_MainRenderContext; }

	// functionality
	//---------------
	void CreateMainRenderContext(RenderWindow* const window);

	// window interface
	//------------------
	void OnWindowDestroyed(RenderWindow const* const window) override;

	// Data
	///////

private:
	GraphicsContext m_MainRenderContext;
	// in the future if we need additional graphics contexts we can add them here
};


} // namespace rhi
} // namespace et

