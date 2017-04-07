#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class FrameBuffer;
class TexPBRMaterial;
class EmissiveMaterial;
class PointLight;
class Entity;

struct SwirlyLight
{
	Entity* light;
	PointLight* comp;
	glm::vec3 origin;

	float angle = 0;
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

	PointLight* m_pLight = nullptr;
	Entity* m_pLigEnt = nullptr;
};

