#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class FrameBuffer;
class TexPBRMaterial;
class EmissiveMaterial;
class PointLight;
class Entity;
class Skybox;
class DirectionalLight;
class SpriteFont;

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
	Entity* m_pLigEntity = nullptr;
	DirectionalLight* m_pLight = nullptr;

	SpriteFont* m_pDebugFont = nullptr;
};

