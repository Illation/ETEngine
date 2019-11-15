#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <Engine/SceneGraph/AbstractScene.h>


class PlanetTemplate;
class Entity;
class DirectionalLight;
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
	Entity* m_pLigEntity = nullptr;
	DirectionalLight* m_pLight = nullptr;

	PlanetTemplate* m_pPlanet = nullptr;

	AssetPtr<SpriteFont> m_pDebugFont;
};

