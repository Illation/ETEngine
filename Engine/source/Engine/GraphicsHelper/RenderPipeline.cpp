#include "stdafx.h"
#include "RenderPipeline.h"

#include "LightVolume.h"
#include "ShadowRenderer.h"
#include "RenderState.h"
#include "TextRenderer.h"
#include "PrimitiveRenderer.h"
#include "SpriteRenderer.h"
#include "ScreenSpaceReflections.h"
#include "DebugRenderer.h"
#include "PbrPrefilter.h"
#include "SceneRenderer.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Helper/PerformanceInfo.h>

#include <Engine/Helper/ScreenshotCapture.h>
#include <Engine/PlanetTech/AtmospherePrecompute.h>
#include <Engine/Framebuffers/PostProcessingRenderer.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/SceneGraph/AbstractScene.h>
#include <Engine/SceneGraph/Entity.h>
#include <Engine/Graphics/CIE.h>
#include <Engine/Graphics/SpriteFont.h>


RenderPipeline::RenderPipeline()
{
}
RenderPipeline::~RenderPipeline()
{
	PointLightVolume::DestroyInstance();
	DirectLightVolume::DestroyInstance();
	ShadowRenderer::DestroyInstance();
	TextRenderer::DestroyInstance();
	PerformanceInfo::DestroyInstance();
	PrimitiveRenderer::DestroyInstance();
	SpriteRenderer::DestroyInstance();
	AtmospherePrecompute::DestroyInstance();
	PbrPrefilter::DestroyInstance();
	CIE::DestroyInstance();
	DebugRenderer::DestroyInstance();
	ScreenshotCapture::DestroyInstance();

	SafeDelete(m_pSSR);
	SafeDelete(m_pGBuffer);
	SafeDelete(m_pPostProcessing);
}

void RenderPipeline::Initialize()
{
	//Init renderers
	SceneRenderer::GetInstance()->ShowSplashScreen();

	PointLightVolume::GetInstance();
	DirectLightVolume::GetInstance();

	DebugRenderer::GetInstance()->Initialize();
	ShadowRenderer::GetInstance()->Initialize();

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

	PbrPrefilter::GetInstance()->Precompute(Config::GetInstance()->GetGraphics().PbrBrdfLutSize);

	m_ClearColor = vec3(101.f / 255.f, 114.f / 255.f, 107.f / 255.f)*0.1f;

	Config::GetInstance()->GetWindow().WindowResizeEvent.AddListener( std::bind( &RenderPipeline::OnResize, this ) );
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

	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	//Shadow Mapping
	//**************
	STATE->SetDepthEnabled(true);
	STATE->SetCullEnabled(true);
	STATE->SetFaceCullingMode(GL_FRONT);//Maybe draw two sided materials in seperate pass
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
	STATE->SetViewport(ivec2(0), windowSettings.Dimensions);

	STATE->SetClearColor(vec4(m_ClearColor, 1.f));
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	STATE->SetFaceCullingMode(GL_BACK);
	for (auto pScene : pScenes)
	{
		pScene->Draw();
		for (Entity* pEntity : pScene->m_pEntityVec)
		{
			pEntity->RootDraw();
		}
	}
	STATE->SetCullEnabled(false);
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
		0, 0, windowSettings.Width, windowSettings.Height,
		0, 0, windowSettings.Width, windowSettings.Height,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	//Render Light Volumes
	//STATE->SetStencilEnabled(true); // #todo lightvolume stencil test

	STATE->SetDepthEnabled(false);
	STATE->SetBlendEnabled(true);
	STATE->SetBlendEquation(GL_FUNC_ADD);
	STATE->SetBlendFunction(GL_ONE, GL_ONE);

	STATE->SetCullEnabled(true);
	STATE->SetFaceCullingMode(GL_FRONT);

	for (auto pScene : pScenes)
	{
		auto lightVec = pScene->GetLights(); 
		for (auto Light : lightVec)
		{
			Light->DrawVolume();
		}
	}

	STATE->SetFaceCullingMode(GL_BACK);
	STATE->SetBlendEnabled(false);

	STATE->SetCullEnabled(false);

	//STATE->SetStencilEnabled(false);
	m_pPostProcessing->EnableInput();
	m_pSSR->Draw();
	STATE->BindReadFramebuffer(m_pSSR->GetTargetFBO());
	STATE->BindDrawFramebuffer(m_pPostProcessing->GetTargetFBO());
	glBlitFramebuffer(
		0, 0, windowSettings.Width, windowSettings.Height,
		0, 0, windowSettings.Width, windowSettings.Height,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	STATE->SetDepthEnabled(true);

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

#if defined(EDITOR) || defined(ET_DEBUG)
	DebugRenderer::GetInstance()->Draw();
#endif

	//Draw to default buffer
	STATE->SetDepthEnabled(false);
	if(pScenes.size() > 0)
		m_pPostProcessing->Draw(outFBO, pScenes[0]->GetPostProcessingSettings());

	for (auto pScene : pScenes)
	{
		pScene->PostDraw();
	}

	ScreenshotCapture::GetInstance()->HandleCapture();
}

void RenderPipeline::DrawOverlays()//Called from within postprocessing draw method
{
	SpriteRenderer::GetInstance()->Draw();
	TextRenderer::GetInstance()->Draw();
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