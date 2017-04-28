#pragma once
#include "../StaticDependancies/glad/glad.h"

class AbstractScene;

class RenderPipeline : public Singleton<RenderPipeline>
{
public:
	RenderPipeline();
	virtual ~RenderPipeline();

	void Initialize();

	void DrawShadow();
	void Draw(std::vector<AbstractScene*> pScenes);

private:
	std::vector<AbstractScene*> m_pRenderScenes;

	Gbuffer* m_pGBuffer = nullptr;
	PostProcessingRenderer* m_pPostProcessing = nullptr;
	glm::vec3 m_ClearColor;
};