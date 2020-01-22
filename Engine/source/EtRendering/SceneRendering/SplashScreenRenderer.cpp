#include "stdafx.h"
#include "SplashScreenRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/SpriteFont.h>


namespace et {
namespace render {


//========================
// Splash Screen Renderer
//========================


//--------------------------------------------
// SplashScreenRenderer::Init
//
// Set up the splash screen
//
void SplashScreenRenderer::Init()
{
	RenderingSystems::AddReference();

	m_TextRenderer.Initialize();
	m_SpriteRenderer.Initialize();

	// #todo: make these customizable
	m_SplashBackgroundTex = ResourceManager::Instance()->GetAssetData<TextureData>("Splashscreen.jpg"_hash);
	m_SplashTitleFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Roboto-Bold.ttf"_hash);
	m_SplashRegFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("RobotoCondensed-Regular.ttf"_hash);

	m_Title = "E   T   E N G I N E";
	m_Subtitle = "LOADING";

	m_IsInitialized = true;
}

//--------------------------------------------
// SplashScreenRenderer::Deinit
//
// Remove references to textures etc.
//
void SplashScreenRenderer::Deinit()
{
	m_IsInitialized = false;

	m_SplashBackgroundTex = nullptr;
	m_SplashTitleFont = nullptr;
	m_SplashRegFont = nullptr;
}

//---------------------------------
// SplashScreenRenderer::OnResize
//
void SplashScreenRenderer::OnResize(ivec2 const dim)
{
	m_Dimensions = dim;
}

//---------------------------------
// SplashScreenRenderer::OnRender
//
// Main scene drawing function
//
void SplashScreenRenderer::OnRender(T_FbLoc const targetFb)
{
	if (!m_IsInitialized)
	{
		return;
	}

	m_SpriteRenderer.Draw(m_SplashBackgroundTex.get(), vec2(0));

	int16 titleFontSize = static_cast<int16>(150.f * (static_cast<float>(m_Dimensions.x) / 1440.f));
	ivec2 titleSize = m_TextRenderer.GetTextSize(m_Title, m_SplashTitleFont.get(), titleFontSize);
	m_TextRenderer.SetColor(vec4(1.f));
	m_TextRenderer.SetFont(m_SplashTitleFont.get());
	m_TextRenderer.DrawText(m_Title, etm::vecCast<float>(m_Dimensions / 2 - titleSize / 2), titleFontSize);

	m_TextRenderer.SetFont(m_SplashRegFont.get());
	int16 loadingFontSize = static_cast<int16>(50.f * (static_cast<float>(m_Dimensions.x) / 1440.f));
	ivec2 loadingSize = m_TextRenderer.GetTextSize(m_Subtitle, m_SplashRegFont.get(), loadingFontSize);
	m_TextRenderer.DrawText(m_Subtitle, etm::vecCast<float>(m_Dimensions - ivec2(loadingSize.x + 20, 20)), loadingFontSize);

	m_SpriteRenderer.Draw();
	m_TextRenderer.Draw();
}


} // namespace render
} // namespace et
