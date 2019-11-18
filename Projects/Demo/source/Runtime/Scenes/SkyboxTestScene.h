#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/AbstractScene.h>


class TexPBRMaterial;
class LightComponent;
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
	void DrawForward() {}
	void PostDraw() {}
private:
	TexPBRMaterial* m_pMat = nullptr;
	LightComponent* m_Light = nullptr;

	AssetPtr<SpriteFont> m_pDebugFont;
};

