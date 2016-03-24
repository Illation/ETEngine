#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class FrameBuffer;
class GbufferMaterial;
class EmissiveMaterial;
class PointLight;
class Entity;
class Skybox;

class SkyboxTestScene : public AbstractScene
{
public:
	SkyboxTestScene();
	~SkyboxTestScene();
private:
	void Initialize();
	void Update();
	void Draw();
	void DrawForward();
	void PostDraw();
private:
	GbufferMaterial* m_pMat = nullptr;
	GbufferMaterial* m_pMat2 = nullptr;
	Skybox* m_pSkybox = nullptr;
};

