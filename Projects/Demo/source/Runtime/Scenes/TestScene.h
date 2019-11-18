#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/AbstractScene.h>


class FrameBuffer;
class TexPBRMaterial;
class EmissiveMaterial;
class LightComponent;
class SpriteFont;


struct SwirlyLight
{
	LightComponent* light = nullptr;
	vec3 origin;
	float radius = 1;
	float radius2 = 1;

	float time1 = 0;
	float timeMult1 = 0;

	float time2 = 0;
	float timeMult2 = 0;

	float angle = 0;
	float angle2 = 0;
};

class TestScene : public AbstractScene
{
public:
	TestScene();
	~TestScene();
private:
	void Initialize();
	void Update();
	void Draw();
	void DrawForward();
	void PostDraw();
private:
	TexPBRMaterial* m_pMat = nullptr;
	EmissiveMaterial* m_pLightMat = nullptr;

	std::vector<SwirlyLight> m_Lights;

	LightComponent* m_Light = nullptr;

	AssetPtr<SpriteFont> m_pDebugFont;
};

