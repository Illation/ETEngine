#include "stdafx.h"
#include "SplashScreenRenderer.h"

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/GraphicsTypes/TextureData.h>


namespace et {
namespace rt {


//========================
// Splash Screen Renderer
//========================


//--------------------------------------------
// SplashScreenRenderer::Init
//
// Set up the splash screen
//
void SplashScreenRenderer::Init()
{
	render::RenderingSystems::AddReference();

	m_GuiRenderer.Init(); 
	m_GuiContext.Init("SplashScreen GUI Context", render::Viewport::GetCurrentViewport()->GetDimensions());

	m_IsInitialized = true;
}

//--------------------------------------------
// SplashScreenRenderer::Deinit
//
// Remove references to textures etc.
//
void SplashScreenRenderer::Deinit()
{
	m_IsInitialized = false;

	m_GuiRenderer.Deinit();
}

//--------------------------------------------
// SplashScreenRenderer::SetGuiDocument
//
void SplashScreenRenderer::SetGuiDocument(core::HashString const documentId)
{
	if (documentId.IsEmpty())
	{
		if (m_GuiContext.IsDocumentLoaded())
		{
			m_GuiContext.UnloadDocument();
		}
	}
	else if (!m_GuiContext.IsDocumentLoaded())
	{
		m_GuiContext.LoadDocument(documentId);
	}
}

//---------------------------------
// SplashScreenRenderer::OnResize
//
void SplashScreenRenderer::OnResize(ivec2 const dim)
{
	if (m_IsInitialized)
	{
		m_GuiContext.SetDimensions(dim);
	}
}

//---------------------------------
// SplashScreenRenderer::OnRender
//
// Main scene drawing function
//
void SplashScreenRenderer::OnRender(render::T_FbLoc const targetFb)
{
	if (!m_IsInitialized || !m_GuiContext.IsDocumentLoaded())
	{
		return;
	}

	m_GuiContext.Update();

	render::Viewport const* const viewport = render::Viewport::GetCurrentViewport();
	m_GuiRenderer.RenderContexts(viewport, targetFb, &m_GuiContext, 1u);
}


} // namespace rt
} // namespace et