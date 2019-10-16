#pragma once

#include "OutlineRenderer.h"

#include <EtCore/Helper/Singleton.h>
#include <EtCore/Content/AssetPointer.h>

#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/GraphicsContext/ViewportRenderer.h>


class PostProcessingRenderer;
class ScreenSpaceReflections;


//---------------------------------
// SceneRenderer
//
// Renders a Scene to the viewport - #todo: fully merge with render pipeline
//
class SceneRenderer final : public I_ViewportRenderer
{
	// GlobalAccess
	//---------------
public:
	static SceneRenderer* GetCurrent();

	// construct destruct
	//--------------------
	SceneRenderer() : I_ViewportRenderer() {}
	~SceneRenderer();

	// functionality
	//-----------------------------
	void InitWithSplashScreen();
	void InitRenderingSystems();

	void ShowSplashScreen();
	void HideSplashScreen();

	void DrawOverlays(T_FbLoc const targetFb);
	void DrawShadow();

private:
	void Draw();

	// Viewport Renderer Interface
	//-----------------------------
protected:
	std::type_info const& GetType() const override { return typeid(SceneRenderer); }
	void OnInit() override {}
	void OnDeinit() override {}
	void OnResize(ivec2 const dim) override;
	void OnRender(T_FbLoc const targetFb) override;

	// accessors
	//--------------
public:
	Gbuffer* GetGBuffer() { return m_GBuffer; }
	OutlineRenderer& GetOutlineRenderer() { return m_OutlineRenderer; }
	std::vector<AbstractScene*> const& GetRenderScenes() { return m_RenderScenes; }

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