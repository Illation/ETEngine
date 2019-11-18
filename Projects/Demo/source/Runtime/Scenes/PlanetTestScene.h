#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/AbstractScene.h>


class PlanetTemplate;
class LightComponent;
class SpriteFont;


class PlanetTestScene : public AbstractScene
{
public:
	PlanetTestScene();
	~PlanetTestScene();

private:
	void Initialize();
	void Update();
	void Draw() {}
	void DrawForward() {}
	void PostDraw() {}

private:

	LightComponent* m_Light = nullptr;
	PlanetTemplate* m_Planet = nullptr;
};

