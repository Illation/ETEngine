#include "stdafx.h"
#include "SceneRenderer.h"

#include "TextRenderer.h"
#include "SpriteRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <Engine/SceneGraph/SceneManager.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/SceneGraph/Entity.h>


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
	RenderPipeline::GetInstance()->Draw(activeScenes, targetFb);
}
