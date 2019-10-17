#include "stdafx.h"
#include "SceneRenderer.h"

#include "ScreenSpaceReflections.h"
#include "PostProcessingRenderer.h"
#include "Gbuffer.h"

#include <EtCore/Content/ResourceManager.h>
#include <EtCore/Helper/PerformanceInfo.h>

#include <Engine/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <Engine/SceneGraph/SceneManager.h>
#include <Engine/SceneGraph/Entity.h>
#include <Engine/SceneGraph/AbstractScene.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/Graphics/Material.h>
#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Materials/NullMaterial.h>


//=================
// Scene Renderer
//=================


// static
std::vector<SceneRenderer*> SceneRenderer::s_AllSceneRenderers = std::vector<SceneRenderer*>();


//---------------------------------
// SceneRenderer::GetCurrent
//
// Utility function to retrieve the scene renderer for the currently active viewport
//
SceneRenderer* SceneRenderer::GetCurrent()
{
	I_ViewportRenderer* const viewRenderer = Viewport::GetCurrentViewport()->GetViewportRenderer();
	ET_ASSERT(viewRenderer->GetType() == typeid(SceneRenderer));

	return static_cast<SceneRenderer*>(viewRenderer);
}


//--------------------------------------------------------------------------


//---------------------------------
// SceneRenderer::c-tor
//
SceneRenderer::SceneRenderer() 
	: I_ViewportRenderer()
{
	s_AllSceneRenderers.emplace_back(this);
}

//---------------------------------
// SceneRenderer::d-tor
//
// make sure all the singletons this system requires are uninitialized
//
SceneRenderer::~SceneRenderer()
{
	SafeDelete(m_SSR);
	SafeDelete(m_GBuffer);
	SafeDelete(m_PostProcessing);

	RenderingSystems::RemoveReference();

	// remove this reference from the list of all scene renderers
	auto const foundIt = std::find(s_AllSceneRenderers.begin(), s_AllSceneRenderers.end(), this);
	ET_ASSERT(foundIt != s_AllSceneRenderers.cend());

	s_AllSceneRenderers.erase(foundIt);
}

//---------------------------------
// SceneRenderer::InitWithSplashScreen
//
void SceneRenderer::InitWithSplashScreen()
{
	RenderingSystems::AddReference();

	m_TextRenderer.Initialize();
	m_SpriteRenderer.Initialize();

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

	m_DebugRenderer.Initialize();
	m_ShadowRenderer.Initialize();

	m_OutlineRenderer.Initialize();

	m_PostProcessing = new PostProcessingRenderer();
	m_PostProcessing->Initialize();

	m_GBuffer = new Gbuffer();
	m_GBuffer->Initialize();
	m_GBuffer->Enable(true);

	m_SSR = new ScreenSpaceReflections();
	m_SSR->Initialize();

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

	m_SpriteRenderer.Draw(m_SplashBackgroundTex.get(), vec2(0));

	std::string title = "E   T   E N G I N E";
	int16 titleFontSize = static_cast<int16>(150.f * (static_cast<float>(m_Dimensions.x) / 1440.f));
	ivec2 titleSize = m_TextRenderer.GetTextSize(title, m_SplashTitleFont.get(), titleFontSize);
	m_TextRenderer.SetColor(vec4(1.f));
	m_TextRenderer.SetFont(m_SplashTitleFont.get());
	m_TextRenderer.DrawText(title, etm::vecCast<float>(m_Dimensions / 2 - titleSize / 2), titleFontSize);

	m_TextRenderer.SetFont(m_SplashRegFont.get());
	std::string loading = "LOADING";
	int16 loadingFontSize = static_cast<int16>(50.f * (static_cast<float>(m_Dimensions.x) / 1440.f));
	ivec2 loadingSize = m_TextRenderer.GetTextSize(loading, m_SplashRegFont.get(), loadingFontSize);
	m_TextRenderer.DrawText(loading, etm::vecCast<float>(m_Dimensions - ivec2(loadingSize.x + 20, 20)), loadingFontSize);

	m_SpriteRenderer.Draw();
	m_TextRenderer.Draw();
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
void SceneRenderer::DrawOverlays(T_FbLoc const targetFb)
{
	m_OutlineRenderer.Draw(targetFb);
	m_SpriteRenderer.Draw();
	m_TextRenderer.Draw();
}

//---------------------------------
// SceneRenderer::DrawShadow
//
// Render the scene to the depth buffer of the current framebuffer
//
void SceneRenderer::DrawShadow()
{
	NullMaterial* nullMat = m_ShadowRenderer.GetNullMaterial();

	for (auto scene : m_RenderScenes)
	{
		for (Entity* entity : scene->m_pEntityVec)
		{
			entity->RootDrawMaterial(static_cast<Material*>(nullMat));
		}
	}
}

//---------------------------------
// SceneRenderer::Draw
//
// Main scene drawing function
//
void SceneRenderer::Draw()
{
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
	api->SetViewport(ivec2(0), m_Dimensions);

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
	api->CopyDepthReadToDrawFbo(m_Dimensions, m_Dimensions);

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
	api->CopyDepthReadToDrawFbo(m_Dimensions, m_Dimensions);

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
	m_DebugRenderer->Draw();
#endif

	//Draw to default buffer
	api->SetDepthEnabled(false);
	if (m_RenderScenes.size() > 0)
	{
		m_PostProcessing->Draw(m_TargetFb, m_RenderScenes[0]->GetPostProcessingSettings());
	}

	for (auto pScene : m_RenderScenes)
	{
		pScene->PostDraw();
	}
}

//---------------------------------
// SceneRenderer::OnResize
//
void SceneRenderer::OnResize(ivec2 const dim)
{
	m_Dimensions = dim;

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
	m_TargetFb = targetFb;

	Draw();
}
