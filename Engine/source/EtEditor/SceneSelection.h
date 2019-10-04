#pragma once
#include <vector>

#include "EditorTickOrder.h"

#include <EtCore/UpdateCycle/Tickable.h>

#include <EtEditor/Rendering/EntityIdRenderer.h>


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
	virtual void OnEntitySelectionChanged(Entity* const entity, bool const selected) = 0;
	virtual void OnEntitySelectionCleared() = 0;
};


//--------------------
// SceneSelection
//
class SceneSelection final : public I_Tickable
{
public:

	// ctor dtor
	//---------------
	SceneSelection() : I_Tickable(static_cast<uint32>(E_EditorTickOrder::TICK_SceneSelection)) {}
	~SceneSelection() = default;

	// accessors
	//--------------------
	AbstractScene* GetScene() { return m_Scene; }

	// functionality
	//--------------------
	void SetScene(AbstractScene* const scene);

	void RegisterListener(I_SceneSelectionListener* const listener);
	void UnregisterListener(I_SceneSelectionListener const* const listener);

	void ClearSelection(bool const notify = false);
	void ToggleEntitySelected(Entity* const entity, bool const notify = false);

	void Pick(ivec2 const pos, Viewport* const viewport, bool const add);

private:
	void OnSceneEvent(SceneEventData const* const eventData);

	// I_Tickable interface
	//----------------------
	void OnTick() override; 
	
	// Data
	///////

	AbstractScene* m_Scene = nullptr;
	std::vector<Entity*> m_SelectedEntities;

	std::vector<I_SceneSelectionListener*> m_Listeners;

	vec4 m_OutlineColor = vec4(0.5f, 0.5f, 1.f, 1.f);

	EntityIdRenderer m_IdRenderer;
	bool m_IsIdRendererInitialized = false;
};

