#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/AbstractScene.h>


class LightComponent;
class PlanetComponent;
class AtmosphereComponent;


class PlanetTestScene : public AbstractScene
{
public:
	PlanetTestScene() : AbstractScene("PlanetTestScene") {}
	~PlanetTestScene() = default;

private:
	void Init();
	void Update();

private:

	LightComponent* m_Light = nullptr;

	PlanetComponent* m_Planet = nullptr;
	AtmosphereComponent* m_Atmosphere = nullptr;
};

