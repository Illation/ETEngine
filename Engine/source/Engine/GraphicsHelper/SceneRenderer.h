#pragma once

#include "ViewportRenderer.h"

#include <EtCore/Helper/Singleton.h>
#include <EtCore/Content/AssetPointer.h>

#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Graphics/TextureData.h>


//---------------------------------
// SceneRenderer
//
// Renders a Scene to the viewport - #todo: fully merge with render pipeline
//
class SceneRenderer : public I_ViewportRenderer, public Singleton<SceneRenderer>
{
	friend class Singleton<SceneRenderer>;

	// construct destruct
	//--------------------
private:
	SceneRenderer() : I_ViewportRenderer() {}
	virtual ~SceneRenderer() = default;

	// functionality
	//-----------------------------
public:
	void InitWithSplashScreen();
	void ShowSplashScreen();
	void HideSplashScreen();

	// Viewport Renderer Interface
	//-----------------------------
protected:
	void OnInit() override {}
	void OnDeinit() override {}
	void OnResize(ivec2 const dim) override {}
	void OnRender() override;

	// Data
	///////
private:
	AssetPtr<TextureData> m_SplashBackgroundTex;
	AssetPtr<SpriteFont> m_SplashTitleFont;
	AssetPtr<SpriteFont> m_SplashRegFont;

	bool m_IsShowingSpashScreen = false;
};