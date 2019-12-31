#pragma once
#include <EtFramework/SceneGraph/AbstractScene.h>


class LightComponent;


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
};

