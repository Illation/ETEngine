#pragma once
#include <EtCore/Content/AssetPointer.h>


class AbstractScene;
class RenderState;
class ScreenSpaceReflections;
class TextureData;
class SpriteFont;


class RenderPipeline : public Singleton<RenderPipeline>
{
public:
	RenderPipeline();
	virtual ~RenderPipeline();

	void Initialize();

	Gbuffer* GetGBuffer() { return m_pGBuffer; }
	PostProcessingRenderer* GetPostProcessor() { return m_pPostProcessing; }	

	void DrawShadow();
	void Draw(std::vector<AbstractScene*> pScenes, GLuint outFBO);
	void DrawOverlays();

	void SwapBuffers();

	RenderState* GetState() { return m_pState; }

	void OnResize();

	void ShowSplashScreen();
	void HideSplashScreen();

private:
	std::vector<AbstractScene*> m_pRenderScenes;

	RenderState* m_pState;

	Gbuffer* m_pGBuffer = nullptr;
	PostProcessingRenderer* m_pPostProcessing = nullptr;
	ScreenSpaceReflections* m_pSSR = nullptr;
	vec3 m_ClearColor;

	AssetPtr<TextureData> m_SplashBackgroundTex;
	AssetPtr<SpriteFont> m_SplashTitleFont;
	AssetPtr<SpriteFont> m_SplashRegFont;
};