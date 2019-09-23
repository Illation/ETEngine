#pragma once
#include "UIContainer.h"

#include <EtCore/Content/AssetPointer.h>

//Renderering part in own class so that it can be deleted and reconstructed for resizing
class UIViewportRenderer
{
public:
	UIViewportRenderer() {}
	virtual ~UIViewportRenderer();

	T_FbLoc GetTarget() const { return m_FBO; }
	void Draw(ivec2 pos, ivec2 size);
	void Initialize(ivec2 size);
private:
	AssetPtr<ShaderData> m_pShader;

	bool m_Initialized = false;

	T_FbLoc m_FBO;
	TextureData* m_pTex = nullptr;
};

class UIViewport : public UIFixedContainer
{
public:
	UIViewport();
	virtual ~UIViewport();

	T_FbLoc GetTarget() const { return m_Renderer->GetTarget(); }
	void Initialize();

	void SetSize(ivec2 size)override;

	virtual bool Draw(uint16 level);

private:
	UIViewportRenderer* m_Renderer;
	bool m_RendererInitialized = false;
};