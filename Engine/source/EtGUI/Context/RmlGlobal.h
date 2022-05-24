#pragma once
#include "RmlSystemInterface.h"
#include "RmlFileInterface.h"

#include <EtGUI/Rendering/RmlRenderInterface.h>


namespace et {
namespace gui {


//---------------------------------
// RmlGlobal
//
// Singleton that manages global access into the RmlUi library
//
class RmlGlobal final
{
	// definitions
	//-------------
private:
	friend class RefPtr<RmlGlobal>;
	friend class Create<RmlGlobal>;

	static RefPtr<RmlGlobal> s_Instance;

	// singleton access
	//------------------
public:
	static RefPtr<RmlGlobal> GetInstance();
	static void Destroy();
	static bool IsInitialized();

	// construct destruct
	//--------------------
private:
	RmlGlobal();
	~RmlGlobal();

	// functionality
	//---------------
public:
	void SetCursorShapeManager(Ptr<core::I_CursorShapeManager> const cursorMan);
	void SetGraphicsContext(Ptr<render::I_GraphicsContextApi> const graphicsContext);

	// Data
	///////
	
private:
	RmlSystemInterface m_SystemInterface;
	RmlFileInterface m_FileInterface;
	RmlRenderInterface m_RenderInterface;
};


} // namespace gui
} // namespace et
