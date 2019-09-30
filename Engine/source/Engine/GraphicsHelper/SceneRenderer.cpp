#include "stdafx.h"
#include "SceneRenderer.h"

#include "TextRenderer.h"
#include "SpriteRenderer.h"
#include "LightVolume.h"
#include "ShadowRenderer.h"
#include "PrimitiveRenderer.h"
#include "ScreenSpaceReflections.h"
#include "DebugRenderer.h"
#include "PbrPrefilter.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Helper/PerformanceInfo.h>

#include <Engine/SceneGraph/SceneManager.h>
#include <Engine/SceneGraph/Entity.h>
#include <Engine/SceneGraph/AbstractScene.h>
#include <Engine/Helper/ScreenshotCapture.h>
#include <Engine/PlanetTech/AtmospherePrecompute.h>
#include <Engine/Framebuffers/PostProcessingRenderer.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/Graphics/CIE.h>
#include <Engine/Graphics/SpriteFont.h>


//---------------------------------
// SceneRenderer::SceneRenderer
//
// make sure all the singletons this system requires are uninitialized
//
SceneRenderer::~SceneRenderer()
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

	SafeDelete(m_SSR);
	SafeDelete(m_GBuffer);
	SafeDelete(m_PostProcessing);
}

//---------------------------------
// SceneRenderer::InitWithSplashScreen
//
void SceneRenderer::InitWithSplashScreen()
{
	TextRenderer::GetInstance()->Initialize();
	SpriteRenderer::GetInstance()->Initialize();

	ShowSplashScreen();
}

//---------------------------------
// SceneRenderer::InitRenderingSystems
//
// Create required buffers and subrendering systems etc
//
void SceneRenderer::InitRenderingSystems()
{
	ShowSplashScreen();

	PointLightVolume::GetInstance();
	DirectLightVolume::GetInstance();

	DebugRenderer::GetInstance()->Initialize();
	ShadowRenderer::GetInstance()->Initialize();

	PerformanceInfo::GetInstance();
	PrimitiveRenderer::GetInstance();

	CIE::GetInstance()->LoadData();

	m_PostProcessing = new PostProcessingRenderer();
	m_PostProcessing->Initialize();

	m_GBuffer = new Gbuffer();
	m_GBuffer->Initialize();
	m_GBuffer->Enable(true);

	m_SSR = new ScreenSpaceReflections();
	m_SSR->Initialize();

	PbrPrefilter::GetInstance()->Precompute(Config::GetInstance()->GetGraphics().PbrBrdfLutSize);

	m_ClearColor = vec3(200.f / 255.f, 114.f / 255.f, 200.f / 255.f)*0.0f;

	m_IsInitialized = true;
}

//---------------------------------
// SceneRenderer::ShowSplashScreen
//
// Draw a texture and image while the level is loading
//
void SceneRenderer::ShowSplashScreen()
{
	m_IsShowingSpashScreen = true;

	if (m_SplashBackgroundTex == nullptr)
	{
		m_SplashBackgroundTex = ResourceManager::Instance()->GetAssetData<TextureData>("Splashscreen.jpg"_hash);
	}

	if (m_SplashTitleFont == nullptr)
	{
		m_SplashTitleFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Roboto-Bold.ttf"_hash);
	}

	if (m_SplashRegFont == nullptr)
	{
		m_SplashRegFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("RobotoCondensed-Regular.ttf"_hash);
	}

	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	SpriteRenderer::GetInstance()->Draw(m_SplashBackgroundTex.get(), vec2(0));

	std::string title = "E   T   E N G I N E";
	int16 titleFontSize = static_cast<int16>(150.f * (static_cast<float>(windowSettings.Height) / 1440.f));
	ivec2 titleSize = TextRenderer::GetInstance()->GetTextSize(title, m_SplashTitleFont.get(), titleFontSize);
	TextRenderer::GetInstance()->SetColor(vec4(1.f));
	TextRenderer::GetInstance()->SetFont(m_SplashTitleFont.get());
	TextRenderer::GetInstance()->DrawText(title, etm::vecCast<float>(windowSettings.Dimensions / 2 - titleSize / 2), titleFontSize);

	TextRenderer::GetInstance()->SetFont(m_SplashRegFont.get());
	std::string loading = "LOADING";
	int16 loadingFontSize = static_cast<int16>(50.f * (static_cast<float>(windowSettings.Height) / 1440.f));
	ivec2 loadingSize = TextRenderer::GetInstance()->GetTextSize(loading, m_SplashRegFont.get(), loadingFontSize);
	TextRenderer::GetInstance()->DrawText(loading, etm::vecCast<float>(windowSettings.Dimensions - ivec2(loadingSize.x + 20, 20)), loadingFontSize);

	SpriteRenderer::GetInstance()->Draw();
	TextRenderer::GetInstance()->Draw();
}

//---------------------------------
// SceneRenderer::HideSplashScreen
//
void SceneRenderer::HideSplashScreen()
{
	m_SplashBackgroundTex = nullptr;
	m_SplashTitleFont = nullptr;
	m_SplashRegFont = nullptr;

	m_IsShowingSpashScreen = false;
}

//---------------------------------
// SceneRenderer::DrawOverlays
//
// Post scene things which should be drawn to the viewport
//
void SceneRenderer::DrawOverlays()
{
	SpriteRenderer::GetInstance()->Draw();
	TextRenderer::GetInstance()->Draw();
}

//---------------------------------
// SceneRenderer::DrawShadow
//
// Render the scene to the depth buffer of the current framebuffer
//
void SceneRenderer::DrawShadow()
{
	for (auto pScene : m_RenderScenes)
	{
		for (Entity* pEntity : pScene->m_pEntityVec)
		{
			pEntity->RootDrawShadow();
		}
	}
}

//---------------------------------
// SceneRenderer::Draw
//
// Main scene drawing function
//
void SceneRenderer::Draw(T_FbLoc targetFb)
{
	Config::Settings::Window const& windowSettings = Config::GetInstance()->GetWindow();

	I_GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

	//Shadow Mapping
	//**************
	api->SetDepthEnabled(true);
	api->SetCullEnabled(true);
	api->SetFaceCullingMode(E_FaceCullMode::Front);//Maybe draw two sided materials in seperate pass
	for (auto pScene : m_RenderScenes)
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
	m_GBuffer->Enable();

	//reset viewport
	api->SetViewport(ivec2(0), windowSettings.Dimensions);

	api->SetClearColor(vec4(m_ClearColor, 1.f));
	api->Clear(E_ClearFlag::Color | E_ClearFlag::Depth);

	api->SetFaceCullingMode(E_FaceCullMode::Back);
	for (auto pScene : m_RenderScenes)
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
	m_SSR->EnableInput();
	//Ambient IBL
	m_GBuffer->Draw();

	//copy Z-Buffer from gBuffer
	api->BindReadFramebuffer(m_GBuffer->Get());
	api->BindDrawFramebuffer(m_SSR->GetTargetFBO());
	api->CopyDepthReadToDrawFbo(windowSettings.Dimensions, windowSettings.Dimensions);

	//Render Light Volumes
	//api->SetStencilEnabled(true); // #todo lightvolume stencil test

	api->SetDepthEnabled(false);
	api->SetBlendEnabled(true);
	api->SetBlendEquation(E_BlendEquation::Add);
	api->SetBlendFunction(E_BlendFactor::One, E_BlendFactor::One);

	api->SetCullEnabled(true);
	api->SetFaceCullingMode(E_FaceCullMode::Front);

	for (auto pScene : m_RenderScenes)
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
	m_PostProcessing->EnableInput();
	m_SSR->Draw();
	api->BindReadFramebuffer(m_SSR->GetTargetFBO());
	api->BindDrawFramebuffer(m_PostProcessing->GetTargetFBO());
	api->CopyDepthReadToDrawFbo(windowSettings.Dimensions, windowSettings.Dimensions);

	api->SetDepthEnabled(true);

	//Foreward Rendering
	//******************
	//Step two: render with forward materials
	for (auto pScene : m_RenderScenes)
	{
		if (pScene->m_UseSkyBox)
		{
			pScene->m_pSkybox->RootDrawForward();
		}
	}
	for (auto pScene : m_RenderScenes)
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
	if (m_RenderScenes.size() > 0)
	{
		m_PostProcessing->Draw(targetFb, m_RenderScenes[0]->GetPostProcessingSettings());
	}

	for (auto pScene : m_RenderScenes)
	{
		pScene->PostDraw();
	}

	ScreenshotCapture::GetInstance()->HandleCapture();
}

//---------------------------------
// SceneRenderer::OnResize
//
void SceneRenderer::OnResize(ivec2 const dim)
{
	if (!m_IsInitialized)
	{
		return;
	}

	m_PostProcessing->~PostProcessingRenderer();
	m_PostProcessing = new(m_PostProcessing) PostProcessingRenderer();
	m_PostProcessing->Initialize();
	m_SSR->~ScreenSpaceReflections();
	m_SSR = new(m_SSR) ScreenSpaceReflections();
	m_SSR->Initialize();
}

//---------------------------------
// SceneRenderer::OnRender
//
// Draw the currently active scnes
//
void SceneRenderer::OnRender(T_FbLoc const targetFb)
{
	if (m_IsShowingSpashScreen)
	{
		ShowSplashScreen();
		return;
	}

	std::vector<AbstractScene*> activeScenes;
	if (SceneManager::GetInstance()->GetActiveScene())
	{
		activeScenes.push_back(SceneManager::GetInstance()->GetActiveScene());
	}

	if (activeScenes.empty())
	{
		return;
	}

	// #note: currently only one scene but could be expanded for nested scenes
	m_RenderScenes = activeScenes;

	Draw(targetFb);
}
