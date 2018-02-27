#include "stdafx.hpp"

#include "RenderPipeline.hpp"
#include "../SceneGraph/AbstractScene.hpp"
#include "LightVolume.hpp"
#include "ShadowRenderer.hpp"
#include "RenderState.hpp"
#include "TextRenderer.hpp"
#include "../Helper/PerformanceInfo.hpp"
#include "PrimitiveRenderer.hpp"
#include "../Framebuffers/PostProcessingRenderer.hpp"
#include "../Framebuffers/Gbuffer.hpp"
#include "../Components/LightComponent.hpp"
#include "../SceneGraph/Entity.hpp"
#include "../Prefabs/Skybox.hpp"
#include "SpriteRenderer.hpp"
#include "AtmospherePrecompute.h"
#include "PbrPrefilter.h"
#include "CIE.h"
#include "ScreenSpaceReflections.h"
#include "DebugRenderer.h"
#include "ScreenshotCapture.h"

RenderPipeline::RenderPipeline()
{
}
RenderPipeline::~RenderPipeline()
{
	PointLightVolume::GetInstance()->DestroyInstance();
	DirectLightVolume::GetInstance()->DestroyInstance();
	ShadowRenderer::GetInstance()->DestroyInstance();
	TextRenderer::GetInstance()->DestroyInstance();
	PerformanceInfo::GetInstance()->DestroyInstance();
	PrimitiveRenderer::GetInstance()->DestroyInstance();
	SpriteRenderer::GetInstance()->DestroyInstance();
	AtmospherePrecompute::GetInstance()->DestroyInstance();
	PbrPrefilter::GetInstance()->DestroyInstance();
	CIE::GetInstance()->DestroyInstance();
	DebugRenderer::GetInstance()->DestroyInstance();
	ScreenshotCapture::GetInstance()->DestroyInstance();

	SafeDelete(m_pSSR);
	SafeDelete(m_pGBuffer);
	SafeDelete(m_pPostProcessing);
	SafeDelete(m_pState);
}

void RenderPipeline::Initialize()
{
	//Init renderers
	m_pState = new RenderState();
	m_pState->Initialize();

	PointLightVolume::GetInstance();
	DirectLightVolume::GetInstance();

	DebugRenderer::GetInstance()->Initialize();
	ShadowRenderer::GetInstance()->Initialize();
	TextRenderer::GetInstance()->Initialize();
	SpriteRenderer::GetInstance()->Initialize();

	PerformanceInfo::GetInstance();
	PrimitiveRenderer::GetInstance();

	CIE::GetInstance()->LoadData();

	m_pPostProcessing = new PostProcessingRenderer();
	m_pPostProcessing->Initialize();

	m_pGBuffer = new Gbuffer();
	m_pGBuffer->Initialize();
	m_pGBuffer->Enable(true);

	m_pSSR = new ScreenSpaceReflections();
	m_pSSR->Initialize();

	PbrPrefilter::GetInstance()->Precompute(GRAPHICS.PbrBrdfLutSize);

	m_ClearColor = vec3(101.f / 255.f, 114.f / 255.f, 107.f / 255.f)*0.1f;

	WINDOW.WindowResizeEvent.AddListener( std::bind( &RenderPipeline::OnResize, this ) );
}

void RenderPipeline::DrawShadow()
{
	for (auto pScene : m_pRenderScenes)
	{
		for (Entity* pEntity : pScene->m_pEntityVec)
		{
			pEntity->RootDrawShadow();
		}
	}
}

void RenderPipeline::Draw(std::vector<AbstractScene*> pScenes, GLuint outFBO)
{
	m_pRenderScenes = pScenes;
	//Shadow Mapping
	//**************
	m_pState->SetDepthEnabled(true);
	m_pState->SetCullEnabled(true);
	m_pState->SetFaceCullingMode(GL_FRONT);//Maybe draw two sided materials in seperate pass
	for (auto pScene : pScenes)
	{
		auto lightVec = pScene->GetLights(); //Todo: automatically add all light components to an array for faster access
		for (auto Light : lightVec)
		{
			Light->GenerateShadow();
		}
	}

	//Deferred Rendering
	//******************
	//Step one: Draw the data onto gBuffer
	m_pGBuffer->Enable();

	//reset viewport
	m_pState->SetViewport(ivec2(0), WINDOW.Dimensions);

	m_pState->SetClearColor(vec4(m_ClearColor, 1.f));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_pState->SetFaceCullingMode(GL_BACK);
	for (auto pScene : pScenes)
	{
		pScene->Draw();
		for (Entity* pEntity : pScene->m_pEntityVec)
		{
			pEntity->RootDraw();
		}
	}
	m_pState->SetCullEnabled(false);
	//Step two: blend data and calculate lighting with gbuffer
	//STATE->BindFramebuffer( 0 );
	//m_pPostProcessing->EnableInput();
	m_pSSR->EnableInput();
	//Ambient IBL
	m_pGBuffer->Draw();

	//copy Z-Buffer from gBuffer
	STATE->BindReadFramebuffer(m_pGBuffer->Get());
	STATE->BindDrawFramebuffer(m_pSSR->GetTargetFBO());
	//STATE->BindDrawFramebuffer( 0 );
	glBlitFramebuffer(
		0, 0, WINDOW.Width, WINDOW.Height,
		0, 0, WINDOW.Width, WINDOW.Height,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	//Render Light Volumes
	//STATE->SetStencilEnabled(true); // #todo lightvolume stencil test

	m_pState->SetDepthEnabled(false);
	m_pState->SetBlendEnabled(true);
	m_pState->SetBlendEquation(GL_FUNC_ADD);
	m_pState->SetBlendFunction(GL_ONE, GL_ONE);

	m_pState->SetCullEnabled(true);
	m_pState->SetFaceCullingMode(GL_FRONT);

	for (auto pScene : pScenes)
	{
		auto lightVec = pScene->GetLights(); 
		for (auto Light : lightVec)
		{
			Light->DrawVolume();
		}
	}

	m_pState->SetFaceCullingMode(GL_BACK);
	m_pState->SetBlendEnabled(false);

	m_pState->SetCullEnabled(false);

	//STATE->SetStencilEnabled(false);
	m_pPostProcessing->EnableInput();
	m_pSSR->Draw();
	STATE->BindReadFramebuffer(m_pSSR->GetTargetFBO());
	STATE->BindDrawFramebuffer(m_pPostProcessing->GetTargetFBO());
	glBlitFramebuffer(
		0, 0, WINDOW.Width, WINDOW.Height,
		0, 0, WINDOW.Width, WINDOW.Height,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	m_pState->SetDepthEnabled(true);

	//Foreward Rendering
	//******************
	//Step two: render with forward materials
	for (auto pScene : pScenes)
	{
		if (pScene->m_UseSkyBox)
		{
			pScene->m_pSkybox->RootDrawForward();
		}
	}
	for (auto pScene : pScenes)
	{
		pScene->DrawForward();
		for (Entity* pEntity : pScene->m_pEntityVec)
		{
			pEntity->RootDrawForward();
		}
	}

#if defined(EDITOR) || defined(_DEBUG)
	DebugRenderer::GetInstance()->Draw();
#endif

	//Draw to default buffer
	m_pState->SetDepthEnabled(false);
	if(pScenes.size() > 0)
		m_pPostProcessing->Draw(outFBO, pScenes[0]->GetPostProcessingSettings());

	SpriteRenderer::GetInstance()->Draw();
	TextRenderer::GetInstance()->Draw();

	for (auto pScene : pScenes)
	{
		pScene->PostDraw();
	}

	ScreenshotCapture::GetInstance()->HandleCapture();

	PERFORMANCE->Update();
}

void RenderPipeline::SwapBuffers()
{
	SDL_GL_SwapWindow(SETTINGS->Window.pWindow);
}

void RenderPipeline::OnResize()
{
	m_pPostProcessing->~PostProcessingRenderer();
	m_pPostProcessing = new(m_pPostProcessing) PostProcessingRenderer();
	m_pPostProcessing->Initialize();
	m_pSSR->~ScreenSpaceReflections();
	m_pSSR = new(m_pSSR) ScreenSpaceReflections();
	m_pSSR->Initialize();
}
