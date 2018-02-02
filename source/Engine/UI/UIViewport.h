#pragma once
#include "UIContainer.h"

//Renderering part in own class so that it can be deleted and reconstructed for resizing
class UIViewportRenderer
{
public:
	UIViewportRenderer() {}
	virtual ~UIViewportRenderer();

	GLuint GetTarget() const { return m_FBO; }
	void Draw(ivec2 pos, ivec2 size);
	void Initialize(ivec2 size);
private:
	ShaderData* m_pShader = nullptr;

	bool m_Initialized = false;

	GLuint m_FBO;
	TextureData* m_pTex = nullptr;
};

class UIViewport : public UIFixedContainer
{
public:
	UIViewport();
	virtual ~UIViewport();

	GLuint GetTarget() const { return m_Renderer->GetTarget(); }
	void Initialize();

	void SetSize(ivec2 size)override;

	virtual bool Draw(uint16 level);

private:
	UIViewportRenderer* m_Renderer;
	bool m_RendererInitialized = false;
};