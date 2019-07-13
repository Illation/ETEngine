#pragma once

#include <Engine/SceneGraph/AbstractScene.h>


class Planet;
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
	void Draw();
	void DrawForward();
	void PostDraw();
private:
	Entity* m_pLigEntity = nullptr;
	DirectionalLight* m_pLight = nullptr;

	Planet* m_pPlanet = nullptr;

	SpriteFont* m_pDebugFont = nullptr;
};

