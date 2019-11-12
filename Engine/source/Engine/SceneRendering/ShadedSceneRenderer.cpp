#include "stdafx.h"
#include "ShadedSceneRenderer.h"

#include <Engine/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <Engine/GraphicsHelper/RenderScene.h>


namespace render {


//=======================
// Shaded Scene Renderer
//=======================


//---------------------------------
// ShadedSceneRenderer::c-tor
//
ShadedSceneRenderer::ShadedSceneRenderer(render::Scene const* const renderScene)
	: I_ViewportRenderer()
	, m_RenderScene(renderScene)
{ }

//---------------------------------
// ShadedSceneRenderer::d-tor
//
// make sure all the singletons this system requires are uninitialized
//
ShadedSceneRenderer::~ShadedSceneRenderer()
{
	RenderingSystems::RemoveReference();
}

//-------------------------------------------
// ShadedSceneRenderer::InitRenderingSystems
//
// Create required buffers and subrendering systems etc
//
void ShadedSceneRenderer::InitRenderingSystems()
{
	RenderingSystems::AddReference();

	m_PostProcessing.Initialize();

	m_GBuffer.Initialize();
	m_GBuffer.Enable(true);

	m_ClearColor = vec3(200.f / 255.f, 114.f / 255.f, 200.f / 255.f)*0.0f;

	m_IsInitialized = true;
}

//---------------------------------
// ShadedSceneRenderer::OnResize
//
void ShadedSceneRenderer::OnResize(ivec2 const dim)
{
	m_Dimensions = dim;

	if (!m_IsInitialized)
	{
		return;
	}

	m_PostProcessing = PostProcessingRenderer();
	m_PostProcessing.Initialize();
}

//---------------------------------
// ShadedSceneRenderer::OnRender
//
// Main scene drawing function
//
void ShadedSceneRenderer::OnRender(T_FbLoc const targetFb)
{
	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Deferred Rendering
	//******************
	//Step one: Draw the data onto gBuffer
	m_GBuffer.Enable();

	//reset viewport
	api->SetViewport(ivec2(0), m_Dimensions);
	api->SetDepthEnabled(true);
	api->SetCullEnabled(true);

	api->SetClearColor(vec4(m_ClearColor, 1.f));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

	// #todo: render opaque objects to the GBuffer

	// render ambient IBL
	api->SetFaceCullingMode(E_FaceCullMode::Back);
	api->SetCullEnabled(false);

	m_PostProcessing.EnableInput();
	m_GBuffer.Draw();

	//copy Z-Buffer from gBuffer
	api->BindReadFramebuffer(m_GBuffer.Get());
	api->BindDrawFramebuffer(m_PostProcessing.GetTargetFBO());
	api->CopyDepthReadToDrawFbo(m_Dimensions, m_Dimensions);

	// Render Light Volumes
	api->SetDepthEnabled(false);
	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);

	api->SetCullEnabled(true);
	api->SetFaceCullingMode(E_FaceCullMode::Front);

	// #todo: light rendering here

	api->SetFaceCullingMode(E_FaceCullMode::Back);
	api->SetBlendEnabled(false);

	// post processing
	api->SetCullEnabled(false);
	m_PostProcessing.Draw(targetFb, m_RenderScene->GetPostProcessingSettings());
}


} // namespace render
