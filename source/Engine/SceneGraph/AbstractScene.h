#pragma once
#include "../Graphics/PostProcessingSettings.hpp"
//forward declaration
class Entity;
class CameraComponent;
class LightComponent;
class Time;
class ContextObjects;
class SceneManager;
class Gbuffer;
class PostProcessingRenderer;
class AudioListenerComponent;
class Skybox;
class CubeMap;
class HDRMap;
class PhysicsWorld;

class AbstractScene
{
public:
	AbstractScene(std::string name);
	virtual ~AbstractScene();

	void AddEntity(Entity* pEntity);
	void RemoveEntity(Entity* pEntity, bool deleteEntity = true);
	void SetActiveCamera(CameraComponent* pCamera);
	void SetSkybox(std::string assetFile);
	HDRMap* GetEnvironmentMap();
	Skybox* GetSkybox() { return m_pSkybox; }
	std::vector<LightComponent*> GetLights();
	const PostProcessingSettings& GetPostProcessingSettings() const;

	bool SkyboxEnabled() { return m_UseSkyBox; }

	PhysicsWorld* GetPhysicsWorld() const { return m_pPhysicsWorld; }

	AudioListenerComponent* GetAudioListener() const { return m_AudioListener; }
	void SetAudioListener(AudioListenerComponent* val) { m_AudioListener = val; }
protected:

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void DrawForward() = 0;
	virtual void PostDraw() = 0;
	virtual void OnActivated() {}
	virtual void OnDeactivated() {}

	PostProcessingSettings m_PostProcessingSettings;

private:
	friend class SceneManager;
	friend class RenderPipeline;

	void RootInitialize();
	void RootUpdate();
	void RootOnActivated();
	void RootOnDeactivated();

	bool m_IsInitialized = false;
	std::string m_Name;
	std::vector<Entity*> m_pEntityVec;
	CameraComponent *m_pDefaultCam = nullptr;
	Time *m_pTime = nullptr;
	ContextObjects* m_pConObj = nullptr;

	AudioListenerComponent* m_AudioListener = nullptr;

	PhysicsWorld* m_pPhysicsWorld = nullptr;

	bool m_UseSkyBox = false;
	Skybox* m_pSkybox = nullptr;
};

