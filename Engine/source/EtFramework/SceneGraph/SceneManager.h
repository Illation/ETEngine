#pragma once
#include "SceneEvents.h"

#include <EtCore/Helper/Singleton.h>
#include <EtCore/UpdateCycle/Tickable.h>

#include <EtRendering/SceneStructure/RenderScene.h>

#include <EtFramework/Config/TickOrder.h>


class AbstracScene;


//--------------------
// SceneManager
//
// Global class responsible for controlling the current scene, switching between scenes - also provides access to the render scene
//
class SceneManager : public Singleton<SceneManager>, public I_Tickable
{
	// construct destruct
	//--------------------
public:
	SceneManager() : I_Tickable(static_cast<uint32>(E_TickOrder::TICK_SceneManager)) {}
	~SceneManager();

	// functionality
	//---------------
	void AddScene(AbstractScene* const scene);
	void RemoveScene(AbstractScene* const scene);

	void SetActiveGameScene(std::string const& sceneName);

	void NextScene();
	void PreviousScene();

	// accessors
	//-----------
	AbstractScene* GetActiveScene() const { return m_ActiveScene; }
	AbstractScene* GetNewActiveScene() const { return m_NewActiveScene; }

	render::Scene& GetRenderScene() { return m_RenderScene; }

	T_SceneEventDispatcher& GetEventDispatcher() { return m_EventDispatcher; }

	// tickable interface
	//--------------------
protected:
	void OnTick() override;
	
	// utility
	//---------
private:
	void SetNewScene(AbstractScene* const scene);

	
	// Data
	///////

	std::vector<AbstractScene*> m_Scenes;

	AbstractScene* m_ActiveScene = nullptr;
	AbstractScene* m_NewActiveScene = nullptr;

	render::Scene m_RenderScene;

	T_SceneEventDispatcher m_EventDispatcher;
};

