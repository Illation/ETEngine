#pragma once

class AbstractScene;
class ScreenSpaceReflections;


class RenderPipeline : public Singleton<RenderPipeline>
{
public:
	RenderPipeline();
	virtual ~RenderPipeline();

	void Initialize();

	Gbuffer* GetGBuffer() { return m_pGBuffer; }
	PostProcessingRenderer* GetPostProcessor() { return m_pPostProcessing; }	

	void DrawShadow();
	void Draw(std::vector<AbstractScene*> pScenes, uint32 outFBO);
	void DrawOverlays();

	void OnResize();

private:
	std::vector<AbstractScene*> m_pRenderScenes;

	Gbuffer* m_pGBuffer = nullptr;
	PostProcessingRenderer* m_pPostProcessing = nullptr;
	ScreenSpaceReflections* m_pSSR = nullptr;
	vec3 m_ClearColor;
};