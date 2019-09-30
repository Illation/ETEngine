#pragma once
#include <vector>

class AbstractScene;
class Entity;


//-------------------------------
// I_SceneSelectionListener
//
// interface for a class that listens for changes in the scene selection
//
class I_SceneSelectionListener
{
public:
	virtual ~I_SceneSelectionListener() = default;

	virtual void OnSceneEvent(SceneEventData const* const) = 0;
};


//--------------------
// SceneSelection
//
class SceneSelection
{
public:

	// accessors
	//--------------------
	AbstractScene* GetScene() { return m_Scene; }

	// functionality
	//--------------------
	void SetScene(AbstractScene* const scene);

	void RegisterListener(I_SceneSelectionListener* const listener);
	void UnregisterListener(I_SceneSelectionListener const* const listener);

	void ClearSelection();
	void AddItemToSelection(Entity* const entity);

private:
	void OnSceneEvent(SceneEventData const* const eventData);

	// Data
	///////

	AbstractScene* m_Scene = nullptr;
	std::vector<Entity*> m_SelectedEntities;

	std::vector<I_SceneSelectionListener*> m_Listeners;
};

