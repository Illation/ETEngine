#pragma once
#include <string>
#include <vector>

//forward declaration
class Entity;
class Camera;
class Time;
class ContextObjects;
class SceneManager;

class AbstractScene
{
public:
	AbstractScene(std::string name);
	virtual ~AbstractScene();

	void AddEntity(Entity* pEntity);
	void RemoveEntity(Entity* pEntity, bool deleteEntity = true);
	void SetActiveCamera(Camera* pCamera);

protected:

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
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
	Camera *m_pDefaultCam = nullptr;
	Time *m_pTime = nullptr;
	ContextObjects* m_pConObj = nullptr;
};

