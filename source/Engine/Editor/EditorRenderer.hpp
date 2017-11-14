#pragma once
#ifdef EDITOR

class EditorRenderer
{
public:

	EditorRenderer();
	virtual ~EditorRenderer();

	void Initialize();

	void Draw(bool redrawUI);

	GLuint GetSceneTarget();

private:
	void DrawUI();
	void CreateFramebuffers();

	ShaderData* m_EditorUIShader = nullptr;

	GLuint m_SceneFBO;
	TextureData* m_SceneTex = nullptr;

	GLuint m_UIFBO;
	TextureData* m_UITex = nullptr;
};

#endif