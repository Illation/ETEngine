#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class FrameBuffer;
class TexPBRMaterial;
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
	TexPBRMaterial* m_pMat = nullptr;
};

