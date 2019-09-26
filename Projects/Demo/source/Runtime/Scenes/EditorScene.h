#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/SceneGraph/AbstractScene.h>


class Entity;
class DirectionalLight;
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
	void Draw();
	void DrawForward() {}
	void PostDraw() {}

private:

	UberMaterial* m_Mat = nullptr;
	UberMaterial* m_FloorMat = nullptr;
	Entity* m_pLigEntity = nullptr;
	DirectionalLight* m_pLight = nullptr;

	AssetPtr<SpriteFont> m_pDebugFont;
};

