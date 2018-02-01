#pragma once
#include "UIContainer.h"

class UIViewport : public UIContainer
{
public:
	UIViewport();
	virtual ~UIViewport();

	GLuint GetTarget() const;
	void Initialize();

	virtual bool Draw(uint16 level);
	iRect CalculateDimensions(const ivec2 &worldPos);

	void SetSize(ivec2 size);
	ivec2 GetSize() const { return m_Rect.size; }

private:
	ShaderData* m_pShader = nullptr;

	bool m_HasFBO = false;

	GLuint m_FBO;
	TextureData* m_pTex = nullptr;
	void CreateFramebuffers();
};