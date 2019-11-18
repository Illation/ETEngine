#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/AbstractScene.h>


class LightComponent;
class SpriteFont;
class UberMaterial;


class EditorScene final : public AbstractScene
{
public:
	EditorScene() : AbstractScene("EditorScene") {}
	virtual ~EditorScene();

private:
	void Initialize();
	void Update();
	void Draw() {}
	void DrawForward() {}
	void PostDraw() {}

private:

	UberMaterial* m_Mat = nullptr;
	UberMaterial* m_FloorMat = nullptr;
	LightComponent* m_Light = nullptr;

	AssetPtr<SpriteFont> m_pDebugFont;
};

