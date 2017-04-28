#include "stdafx.hpp"

#include "RenderPipeline.hpp"
#include "../SceneGraph/AbstractScene.hpp"
#include "LightVolume.hpp"
#include "ShadowRenderer.hpp"
#include "TextRenderer.h"
#include "../Helper/PerformanceInfo.hpp"
#include "PrimitiveRenderer.hpp"
#include "../Framebuffers/PostProcessingRenderer.hpp"
#include "../Framebuffers/Gbuffer.hpp"
#include "../Components/LightComponent.hpp"
#include "../SceneGraph/Entity.hpp"
#include "../Prefabs/Skybox.hpp"

//Abstract
//*********

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

	SafeDelete(m_pGBuffer);
	SafeDelete(m_pPostProcessing);
}

void RenderPipeline::Initialize()
{
	//Init renderers
	PointLightVolume* pVol = PointLightVolume::GetInstance();
	DirectLightVolume* pDirVol = DirectLightVolume::GetInstance();
	ShadowRenderer* pShadowRenderer = ShadowRenderer::GetInstance();
	pShadowRenderer->Initialize();
	TextRenderer* pTextRenderer = TextRenderer::GetInstance();
	pTextRenderer->Initialize();
	PerformanceInfo* pInfo = PerformanceInfo::GetInstance();
	auto primRenderer = PrimitiveRenderer::GetInstance();

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
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);//Maybe draw two sided materials in seperate pass
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
	int width = SETTINGS->Window.Width, height = SETTINGS->Window.Height;
	glViewport(0, 0, width, height);

	glClearColor(m_ClearColor.x, m_ClearColor.y, m_ClearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_BACK);
	for (auto pScene : pScenes)
	{
		pScene->Draw();
		for (Entity* pEntity : pScene->m_pEntityVec)
		{
			pEntity->RootDraw();
		}
	}
	glDisable(GL_CULL_FACE);
	//Step two: blend data and calculate lighting with gbuffer
	m_pPostProcessing->EnableInput();
	//Ambient IBL lighting
	m_pGBuffer->Draw();

	//copy Z-Buffer from gBuffer
	glBindFramebuffer(GL_READ_FRAMEBUFFER, m_pGBuffer->Get());
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pPostProcessing->GetTargetFBO());
	glBlitFramebuffer(
		0, 0, SETTINGS->Window.Width, SETTINGS->Window.Height,
		0, 0, SETTINGS->Window.Width, SETTINGS->Window.Height,
		GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	//Render Light Volumes
	//glEnable(GL_STENCIL_TEST); // #todo lightvolume stencil test

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	for (auto pScene : pScenes)
	{
		auto lightVec = pScene->GetLights(); 
		for (auto Light : lightVec)
		{
			Light->DrawVolume();
		}
	}
	glCullFace(GL_BACK);
	glDisable(GL_BLEND);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	//glDisable(GL_STENCIL_TEST);

	//Foreward Rendering
	//******************
	//Step two: render with forward materials
	for (auto pScene : pScenes)
	{
		pScene->DrawForward();
		for (Entity* pEntity : pScene->m_pEntityVec)
		{
			pEntity->RootDrawForward();
		}
	}
	for (auto pScene : pScenes)
	{
		if (pScene->m_UseSkyBox)
		{
			pScene->m_pSkybox->RootDrawForward();
		}
	}
	//Draw to default buffer
	glDisable(GL_DEPTH_TEST);
	m_pPostProcessing->Draw(0);

	TextRenderer::GetInstance()->Draw();

	for (auto pScene : pScenes)
	{
		pScene->PostDraw();
	}
	
	//Swap front and back buffer
	SDL_GL_SwapWindow(SETTINGS->Window.pWindow);

	PERFORMANCE->Update();
}