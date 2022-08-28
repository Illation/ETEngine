#pragma once
#include "GraphicsContext.h"


#ifdef ET_SHIPPING
#	define ET_CT_RHI_SUPPORT_DEBUG ET_DISABLED
#else
#	define ET_CT_RHI_SUPPORT_DEBUG ET_ENABLED
#endif // ET_SHIPPING


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
	static I_RenderDevice* GetRenderDevice();

	// accessors
	//-----------
	GraphicsContext& GetMainRenderContext() { return m_MainRenderContext; }

	// functionality
	//---------------
	void CreateMainRenderContext(Ptr<RenderWindow> const window);

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

