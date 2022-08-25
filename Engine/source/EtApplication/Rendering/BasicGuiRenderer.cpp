#include "stdafx.h"
#include "BasicGuiRenderer.h"

#include <EtRHI/GraphicsTypes/TextureData.h>
#include <EtRHI/GraphicsTypes/Shader.h>


namespace et {
namespace app {


//====================
// Basic GUI Renderer
//====================


//------------------------
// BasicGuiRenderer::Init
//
void BasicGuiRenderer::Init()
{
	m_GuiRenderer.Init();

	m_IsInitialized = true;
}

//--------------------------------------------
// BasicGuiRenderer::Deinit
//
// Remove references to textures etc.
//
void BasicGuiRenderer::Deinit()
{
	m_IsInitialized = false;

	m_GuiRenderer.Deinit();
}

//----------------------------------
// BasicGuiRenderer::SetGuiDocument
//
void BasicGuiRenderer::SetGuiDocument(core::HashString const documentId)
{
	if (documentId.IsEmpty())
	{
		if (m_GuiContext.GetDocumentCount() > 0u)
		{
			m_GuiContext.UnloadDocument(m_GuiContext.GetDocumentId(0u));
			m_GuiContext.Deinit();
		}
	}
	else if (m_GuiContext.GetDocumentCount() == 0u)
	{
		m_GuiContext.Init("Basic GUI Context", rhi::Viewport::GetCurrentViewport()->GetDimensions());
		m_GuiContext.LoadDocument(documentId);
	}
}

//----------------------------
// BasicGuiRenderer::OnResize
//
void BasicGuiRenderer::OnResize(ivec2 const dim)
{
	if (m_IsInitialized)
	{
		m_GuiContext.SetDimensions(dim);
	}
}

//----------------------------
// BasicGuiRenderer::OnRender
//
// Main scene drawing function
//
void BasicGuiRenderer::OnRender(rhi::T_FbLoc const targetFb)
{
	if (!m_IsInitialized || !m_GuiContext.HasActiveDocuments())
	{
		return;
	}

	m_GuiContext.Update();

	rhi::Viewport const* const viewport = rhi::Viewport::GetCurrentViewport();
	m_ContextRenderTarget.UpdateForDimensions(viewport->GetDimensions());
	m_GuiRenderer.RenderContext(targetFb, m_ContextRenderTarget, m_GuiContext);
}


} // namespace app
} // namespace et

