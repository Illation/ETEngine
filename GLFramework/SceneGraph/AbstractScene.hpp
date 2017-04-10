#pragma once
//forward declaration
class Entity;
class CameraComponent;
class LightComponent;
class Time;
class ContextObjects;
class SceneManager;
class Gbuffer;
class PostProcessingRenderer;
class Skybox;
class CubeMap;
class HDRMap;

class AbstractScene
{
public:
	AbstractScene(std::string name);
	virtual ~AbstractScene();

	void AddEntity(Entity* pEntity);
	void RemoveEntity(Entity* pEntity, bool deleteEntity = true);
	void SetActiveCamera(CameraComponent* pCamera);
	void SetSkybox(string assetFile);
	//CubeMap* GetEnvironmentMap();
	HDRMap* GetEnvironmentMap();
	std::vector<LightComponent*> GetLights();
	Gbuffer* GetGBuffer() { return m_pGBuffer; }

	bool SkyboxEnabled() { return m_UseSkyBox; }

protected:

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
	virtual void DrawForward() = 0;
	virtual void PostDraw() = 0;
	virtual void OnActivated() {}
	virtual void OnDeactivated() {}

private:
	friend class SceneManager;

	void RootInitialize();
	void RootUpdate();
	void RootDraw();
	void RootOnActivated();
	void RootOnDeactivated();

	bool m_IsInitialized = false;
	std::string m_Name;
	std::vector<Entity*> m_pEntityVec;
	CameraComponent *m_pDefaultCam = nullptr;
	Time *m_pTime = nullptr;
	ContextObjects* m_pConObj = nullptr;

	float m_Exposure = 1;
	glm::vec3 m_ClearColor;
	Gbuffer* m_pGBuffer = nullptr;
	Gbuffer* m_pDemoBuffer = nullptr;
	bool m_DemoMode = false;
	PostProcessingRenderer* m_pPostProcessing = nullptr;

	bool m_UseSkyBox = false;
	Skybox* m_pSkybox = nullptr;
};

