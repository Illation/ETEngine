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
	void Init();
	void Update();

private:

	LightComponent* m_Light = nullptr;
	PlanetTemplate* m_Planet = nullptr;
};

