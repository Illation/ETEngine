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

	ShadowRenderer::GetInstance()->Initialize();
	TextRenderer::GetInstance()->Initialize();
	SpriteRenderer::GetInstance()->Initialize();

	PerformanceInfo::GetInstance();
	PrimitiveRenderer::GetInstance();

	m_pPostProcessing = new PostProcessingRenderer();
	m_pPostProcessing->SetGamma(2.2f);
	m_pPostProcessing->SetExposure(1);
	m_pPostProcessing->SetBloomMultiplier(0.1f);
	m_pPostProcessing->SetBloomThreshold(10.0f);
	m_pPostProcessing->Initialize();

	m_pGBuffer = new Gbuffer();
	m_pGBuffer->Initialize();
	m_pGBuffer->Enable(true);

	m_ClearColor = vec3(101.f / 255.f, 114.f / 255.f, 107.f / 255.f)*0.1f;
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

void RenderPipeline::Draw(std::vector<AbstractScene*> pScenes)
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
	m_pPostProcessing->EnableInput();
	//STATE->BindFramebuffer( 0 );
	//Ambient IBL lighting
	m_pGBuffer->Draw();

	//copy Z-Buffer from gBuffer
	STATE->BindReadFramebuffer(m_pGBuffer->Get());
	STATE->BindDrawFramebuffer(m_pPostProcessing->GetTargetFBO());
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

	m_pState->SetDepthEnabled(true);
	m_pState->SetCullEnabled(false);

	//STATE->SetStencilEnabled(false);

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
	//Draw to default buffer
	m_pState->SetDepthEnabled(false);
	m_pPostProcessing->Draw(0);

	SpriteRenderer::GetInstance()->Draw();
	TextRenderer::GetInstance()->Draw();

	for (auto pScene : pScenes)
	{
		pScene->PostDraw();
	}
	
	//Swap front and back buffer
	SDL_GL_SwapWindow(SETTINGS->Window.pWindow);

	PERFORMANCE->Update();
}