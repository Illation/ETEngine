#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/AbstractScene.h>


class TexPBRMaterial;
class PointLight;
class Entity;
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

	AssetPtr<SpriteFont> m_pDebugFont;
};

