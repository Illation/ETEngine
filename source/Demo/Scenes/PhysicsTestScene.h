#pragma once
#include "..\Engine\SceneGraph\AbstractScene.hpp"

class TexPBRMaterial;
class EmissiveMaterial;
class SpriteFont;
class btCollisionShape;

class PhysicsTestScene : public AbstractScene
{
public:
	PhysicsTestScene();
	virtual ~PhysicsTestScene();
private:
	void Initialize();
	void Update();
	void Draw();
	void DrawForward() {}
	void PostDraw() {}
private:
	TexPBRMaterial* m_pFloorMat = nullptr;
	TexPBRMaterial* m_pBallMat = nullptr;
	TexPBRMaterial* m_pBlockMat = nullptr;

	EmissiveMaterial* m_pLightMat = nullptr;

	btCollisionShape* m_pSphereShape = nullptr;
	float m_SphereSize = 0.2f;
	float m_SphereForce = 30;
	float m_SphereMass = 3;

	float m_BlockMass = 0.2f;

	Entity* m_pLightEntity = nullptr;
	vec3 m_LightCentralPos = vec3(0);
	float m_LightRotDistance = 1.f;

	SpriteFont* m_pDebugFont = nullptr;
};