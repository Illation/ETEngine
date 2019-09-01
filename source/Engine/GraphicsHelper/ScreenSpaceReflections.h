#pragma once
#include <EtCore/Content/AssetPointer.h>

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

	AssetPtr<ShaderData> m_pShader;

	GLuint m_CollectFBO;
	TextureData* m_CollectTex = nullptr;
	GLuint m_CollectRBO;
};