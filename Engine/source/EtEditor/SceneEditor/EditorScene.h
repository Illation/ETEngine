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
	~EditorScene() = default;

private:
	void Init();
	void Update();

private:

	LightComponent* m_Light = nullptr;

	AssetPtr<SpriteFont> m_pDebugFont;
};

