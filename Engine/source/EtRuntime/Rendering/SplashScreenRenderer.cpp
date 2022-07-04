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
		if (m_GuiContext.GetDocumentCount() > 0u)
		{
			m_GuiContext.UnloadDocument(m_GuiContext.GetDocumentId(0u));
		}
	}
	else if (m_GuiContext.GetDocumentCount() == 0u)
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
	if (!m_IsInitialized || !m_GuiContext.HasActiveDocuments())
	{
		return;
	}

	m_GuiContext.Update();

	render::Viewport const* const viewport = render::Viewport::GetCurrentViewport();
	m_ContextRenderTarget.UpdateForDimensions(viewport->GetDimensions());
	m_GuiRenderer.RenderContext(targetFb, m_ContextRenderTarget, m_GuiContext);
}


} // namespace rt
} // namespace et
