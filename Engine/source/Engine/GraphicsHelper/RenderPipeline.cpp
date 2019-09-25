#include "stdafx.h"
#include "RenderPipeline.h"

#include "LightVolume.h"
#include "ShadowRenderer.h"
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

	m_ClearColor = vec3(200.f / 255.f, 114.f / 255.f, 200.f / 255.f)*0.8f;

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

void RenderPipeline::Draw(std::vector<AbstractScene*> pScenes, T_FbLoc outFBO)
{
	m_pRenderScenes = pScenes;

	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Shadow Mapping
	//**************
	api->SetDepthEnabled(true);
	api->SetCullEnabled(true);
	api->SetFaceCullingMode(E_FaceCullMode::Front);//Maybe draw two sided materials in seperate pass
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
	api->SetViewport(ivec2(0), windowSettings.Dimensions);

	api->SetClearColor(vec4(m_ClearColor, 1.f));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

	api->SetFaceCullingMode(E_FaceCullMode::Back);
	for (auto pScene : pScenes)
	{
		pScene->Draw();
		for (Entity* pEntity : pScene->m_pEntityVec)
		{
			pEntity->RootDraw();
		}
	}
	api->SetCullEnabled(false);
	//Step two: blend data and calculate lighting with gbuffer
	//m_pPostProcessing->EnableInput();
	m_pSSR->EnableInput();
	//Ambient IBL
	m_pGBuffer->Draw();

	//copy Z-Buffer from gBuffer
	api->BindReadFramebuffer(m_pGBuffer->Get());
	api->BindDrawFramebuffer(m_pSSR->GetTargetFBO());
	api->CopyDepthReadToDrawFbo(windowSettings.Dimensions, windowSettings.Dimensions);

	//Render Light Volumes
	//api->SetStencilEnabled(true); // #todo lightvolume stencil test

	api->SetDepthEnabled(false);
	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);

	api->SetCullEnabled(true);
	api->SetFaceCullingMode(E_FaceCullMode::Front);

	for (auto pScene : pScenes)
	{
		auto lightVec = pScene->GetLights(); 
		for (auto Light : lightVec)
		{
			Light->DrawVolume();
		}
	}

	api->SetFaceCullingMode(E_FaceCullMode::Back);
	api->SetBlendEnabled(false);

	api->SetCullEnabled(false);

	//api->SetStencilEnabled(false);
	m_pPostProcessing->EnableInput();
	m_pSSR->Draw();
	api->BindReadFramebuffer(m_pSSR->GetTargetFBO());
	api->BindDrawFramebuffer(m_pPostProcessing->GetTargetFBO());
	api->CopyDepthReadToDrawFbo(windowSettings.Dimensions, windowSettings.Dimensions);

	api->SetDepthEnabled(true);

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

#if defined(ET_DEBUG)
	DebugRenderer::GetInstance()->Draw();
#endif

	//Draw to default buffer
	api->SetDepthEnabled(false);
	if (pScenes.size() > 0)
	{
		m_pPostProcessing->Draw(outFBO, pScenes[0]->GetPostProcessingSettings());
	}

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