#pragma once

#include "../Helper/Singleton.hpp"
#include <string>
#include <vector>

//Forward Declaration
class AbstractScene;
class AbstractFramework;

class SceneManager : public Singleton<SceneManager>
{
public:
	~SceneManager();

	void AddGameScene(AbstractScene* scene);
	void RemoveGameScene(AbstractScene* scene);
	void SetActiveGameScene(std::string sceneName);
	void NextScene();
	void PreviousScene();
	AbstractScene* GetActiveScene() const { return m_ActiveScene; }
	
private:
	friend class AbstractFramework;
	friend class Singleton<SceneManager>;

	SceneManager();

	void Initialize();
	void Update();

	std::vector<AbstractScene*> m_pSceneVec;
	bool m_IsInitialized = false;
	AbstractScene* m_ActiveScene = nullptr
		, *m_NewActiveScene = nullptr;
};

