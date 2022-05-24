#include "stdafx.h"
#include "RmlGlobal.h"

#include <RmlUi/Core.h>


namespace et {
namespace gui {


//============
// RML Global
//============


RefPtr<RmlGlobal> RmlGlobal::s_Instance = nullptr;


//------------------------
// RmlGlobal::GetInstance
//
// Singleton that manages global access into the RmlUi library 
//  - every RML renderer attached to a viewport should hold a reference to this to manage the lifetime of the library
//
RefPtr<RmlGlobal> RmlGlobal::GetInstance()
{
	if (s_Instance == nullptr)
	{
		s_Instance = Create<RmlGlobal>();
	}

	return s_Instance;
}

//------------------------
// RmlGlobal::GetInstance
//
void RmlGlobal::Destroy()
{
	if (s_Instance != nullptr)
	{
		ET_ASSERT(s_Instance.GetRefCount() == 1, "Singleton destruction won't deinit RmlUi");
		s_Instance = nullptr;
	}
}

//--------------------------
// RmlGlobal::IsInitialized
//
bool RmlGlobal::IsInitialized()
{
	return (s_Instance != nullptr);
}

//----------------------
// RmlGlobal::c-tor
//
// Initializes the RmlUi Library
//
RmlGlobal::RmlGlobal()
{
	Rml::SetSystemInterface(&m_SystemInterface);
	Rml::SetFileInterface(&m_FileInterface);
	Rml::SetRenderInterface(&m_RenderInterface);

	Rml::Initialise();
	LOG("RmlUi loaded\n");
}

//----------------------
// RmlGlobal::d-tor
//
// Deinitializes the RmlUi Library
//
RmlGlobal::~RmlGlobal()
{
	Rml::Shutdown();
	LOG("RmlUi shut down\n");
}

//----------------------------------
// RmlGlobal::SetCursorShapeManager
//
void RmlGlobal::SetCursorShapeManager(Ptr<core::I_CursorShapeManager> const cursorMan)
{
	m_SystemInterface.SetCursorShapeManager(cursorMan);
}

//----------------------------------
// RmlGlobal::SetGraphicsContext
//
void RmlGlobal::SetGraphicsContext(Ptr<render::I_GraphicsContextApi> const graphicsContext)
{
	m_RenderInterface.SetGraphicsContext(graphicsContext);
}


} // namespace gui
} // namespace et

