#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class FrameBuffer;
class GbufferMaterial;
class EmissiveMaterial;
class PointLight;
class Entity;

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
	GbufferMaterial* m_pMat = nullptr;
	EmissiveMaterial* m_pLightMat = nullptr;

	PointLight* m_pLight = nullptr;
	Entity* m_pLigEnt = nullptr;
};

