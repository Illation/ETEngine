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
	Rml::SetFontEngineInterface(&m_FontEngine);
	Rml::SetRenderInterface(&m_Renderer);

	Rml::Initialise();

	m_GlowInstancer.Init();
	Rml::Factory::RegisterFontEffectInstancer("glow", &m_GlowInstancer);

	m_OutlineInstancer.Init();
	Rml::Factory::RegisterFontEffectInstancer("outline", &m_OutlineInstancer);

	m_ShadowInstancer.Init();
	Rml::Factory::RegisterFontEffectInstancer("shadow", &m_ShadowInstancer);

	m_BlurInstancer.Init();
	Rml::Factory::RegisterFontEffectInstancer("blur", &m_BlurInstancer);

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

//-----------------------------------
// RmlGlobal::SetClipboardController
//
void RmlGlobal::SetClipboardController(Ptr<core::I_ClipboardController> const clipboardController)
{
	m_SystemInterface.SetClipboardController(clipboardController);
}

//----------------------------------
// RmlGlobal::SetGraphicsContext
//
void RmlGlobal::SetGraphicsContext(Ptr<render::I_GraphicsContextApi> const graphicsContext)
{
	m_Renderer.SetGraphicsContext(graphicsContext);
}

//----------------------------------
// RmlGlobal::SetGraphicsContext
//
void RmlGlobal::SetRIShader(AssetPtr<render::ShaderData> const shader, AssetPtr<render::ShaderData> const textShader)
{
	m_Renderer.SetShader(shader, textShader);
}

//----------------------------------
// RmlGlobal::SetRIViewDimensions
//
void RmlGlobal::SetRIView(ivec2 const dim, mat4 const& viewProj)
{
	m_Renderer.SetView(dim, viewProj);
}


} // namespace gui
} // namespace et

