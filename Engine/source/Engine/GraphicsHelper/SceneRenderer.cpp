#include "stdafx.h"
#include "SceneRenderer.h"

#include "TextRenderer.h"
#include "SpriteRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/SceneGraph/SceneManager.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>

#ifdef EDITOR
#	include <Engine/Editor/Editor.h>
#endif


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
// SceneRenderer::ShowSplashScreen
//
// Draw a texture and image while the level is loading
//
void SceneRenderer::ShowSplashScreen()
{
	m_IsShowingSpashScreen = true;

	m_SplashBackgroundTex = ResourceManager::Instance()->GetAssetData<TextureData>("Splashscreen.jpg"_hash);
	m_SplashTitleFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Roboto-Bold.ttf"_hash);
	m_SplashRegFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("RobotoCondensed-Regular.ttf"_hash);

	SpriteRenderer::GetInstance()->Draw(m_SplashBackgroundTex.get(), vec2(0));

	std::string title = "E   T   E N G I N E";
	int16 titleFontSize = (int16)(150 * ((float)WINDOW.Height / (float)1440));
	ivec2 titleSize = TextRenderer::GetInstance()->GetTextSize(title, m_SplashTitleFont.get(), titleFontSize);
	TextRenderer::GetInstance()->SetColor(vec4(1));
	TextRenderer::GetInstance()->SetFont(m_SplashTitleFont.get());
	TextRenderer::GetInstance()->DrawText(title, etm::vecCast<float>(WINDOW.Dimensions / 2 - titleSize / 2), titleFontSize);

	TextRenderer::GetInstance()->SetFont(m_SplashRegFont.get());
	std::string loading = "LOADING";
	int16 loadingFontSize = (int16)(50 * ((float)WINDOW.Height / (float)1440));
	ivec2 loadingSize = TextRenderer::GetInstance()->GetTextSize(loading, m_SplashRegFont.get(), loadingFontSize);
	TextRenderer::GetInstance()->DrawText(loading, etm::vecCast<float>(WINDOW.Dimensions - ivec2(loadingSize.x + 20, 20)), loadingFontSize);

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
// SceneRenderer::OnRender
//
// Draw the currently active scnes
//
void SceneRenderer::OnRender()
{
	if (m_IsShowingSpashScreen)
	{
		return;
	}

	std::vector<AbstractScene*> activeScenes;
	if (SceneManager::GetInstance()->GetActiveScene())
	{
		activeScenes.push_back(SceneManager::GetInstance()->GetActiveScene());
	}

#ifdef EDITOR
	RenderPipeline::GetInstance()->Draw(activeScenes, Editor::GetInstance()->GetSceneTarget());
	Editor::GetInstance()->Draw();
#else

	// #note: currently only one scene but could be expanded for nested scenes
	RenderPipeline::GetInstance()->Draw(activeScenes, 0);

#endif
}
