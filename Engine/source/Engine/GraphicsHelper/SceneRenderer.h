#pragma once

#include "ViewportRenderer.h"
#include "OutlineRenderer.h"

#include <EtCore/Helper/Singleton.h>
#include <EtCore/Content/AssetPointer.h>

#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Graphics/TextureData.h>


class PostProcessingRenderer;
class ScreenSpaceReflections;


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
	virtual ~SceneRenderer();

	// functionality
	//-----------------------------
public:
	void InitWithSplashScreen();
	void InitRenderingSystems();

	void ShowSplashScreen();
	void HideSplashScreen();

	void DrawOverlays();
	void DrawShadow();

private:
	void Draw();

	// Viewport Renderer Interface
	//-----------------------------
protected:
	void OnInit() override {}
	void OnDeinit() override {}
	void OnResize(ivec2 const dim) override;
	void OnRender(T_FbLoc const targetFb) override;

	// accessors
	//--------------
public:
	Gbuffer* GetGBuffer() { return m_GBuffer; }
	OutlineRenderer& GetOutlineRenderer() { return m_OutlineRenderer; }

	// Data
	///////
private:

	bool m_IsInitialized = false;

	// splash screen
	AssetPtr<TextureData> m_SplashBackgroundTex;
	AssetPtr<SpriteFont> m_SplashTitleFont;
	AssetPtr<SpriteFont> m_SplashRegFont;

	bool m_IsShowingSpashScreen = false;

	// scene rendering
	vec3 m_ClearColor;

	std::vector<AbstractScene*> m_RenderScenes;
	T_FbLoc m_TargetFb = 0u;

	Gbuffer* m_GBuffer = nullptr;
	PostProcessingRenderer* m_PostProcessing = nullptr;
	ScreenSpaceReflections* m_SSR = nullptr;
	OutlineRenderer m_OutlineRenderer;
};