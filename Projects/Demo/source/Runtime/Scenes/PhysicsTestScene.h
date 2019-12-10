#pragma once
#include <EtCore/Helper/Hash.h>
#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/AbstractScene.h>


class SpriteFont;
class btCollisionShape;
class AudioSourceComponent;
class LightComponent;


class PhysicsTestScene : public AbstractScene
{
public:
	PhysicsTestScene() : AbstractScene("PhysicsTestScene") {}
	virtual ~PhysicsTestScene() = default;
private:
	void Init();
	void Update();

private:
	btCollisionShape* m_pSphereShape = nullptr;
	float m_SphereSize = 0.2f;
	float m_SphereForce = 30;
	float m_SphereMass = 3;

	float m_BlockMass = 0.2f;

	std::vector<T_Hash> m_AudioIdPlaylist;
	size_t m_CurrentTrack = 0u;
	AudioSourceComponent* m_Source;
	LightComponent* m_Light = nullptr;
	vec3 m_LightCentralPos = vec3(0);
	float m_LightRotDistance = 1.f;

	AssetPtr<SpriteFont> m_DebugFont;
};