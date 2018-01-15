#pragma once

class ShaderData;

class ScreenSpaceReflections
{
public:
	ScreenSpaceReflections();
	virtual ~ScreenSpaceReflections();
	void Initialize();

	void EnableInput();
	GLuint GetTargetFBO() { return m_CollectFBO; }

	void Draw();
private:
	void GetUniforms();

	ShaderData* m_pShader = nullptr;

	GLuint m_CollectFBO;
	TextureData* m_CollectTex = nullptr;
	GLuint m_CollectRBO;

	GLint m_uCamPos;

	GLint m_uProjA;
	GLint m_uProjB;
	GLint m_uViewProjInv;
	GLint m_uProjection;
	GLint m_uViewInv;
};