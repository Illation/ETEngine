#pragma once
#include <vector>

#include <EtFramework/SceneGraph/SceneEvents.h>

#include <EtEditor/Util/EditorTickOrder.h>
#include <EtEditor/Rendering/EntityIdRenderer.h>


namespace et { namespace render {
	class OutlineExtension;
} }


namespace et {
namespace edit {


//-------------------------------
// I_SceneSelectionListener
//
// interface for a class that listens for changes in the scene selection
//
class I_SceneSelectionListener
{
public:
	virtual ~I_SceneSelectionListener() = default;

	virtual void OnSceneEvent(fw::E_SceneEvent const, fw::SceneEventData const* const) = 0;
	virtual void OnEntitySelectionChanged(fw::T_EntityId const entity, bool const selected) = 0;
	virtual void OnEntitySelectionCleared() = 0;
};

//--------------------
// SceneSelection
//
class SceneSelection final 
{
public:

	// ctor dtor
	//---------------
	SceneSelection() = default;
	~SceneSelection() = default;

	// accessors
	//--------------------
	std::vector<fw::T_EntityId> const& GetSelectedEntities() const { return m_SelectedEntities; }

	// functionality
	//--------------------
	void SetScene();

	void RegisterListener(I_SceneSelectionListener* const listener);
	void UnregisterListener(I_SceneSelectionListener const* const listener);

	void ClearSelection(bool const notify = false);
	void ToggleEntitySelected(fw::T_EntityId const entity, bool const notify = false);

	void Pick(ivec2 const pos, render::Viewport* const viewport, bool const add);

	void UpdateOutlines() const;
	void RecursiveAddOutlines(fw::T_EntityId const entity) const;

private:
	void OnSceneEvent(fw::T_SceneEventFlags const flags, fw::SceneEventData const* const eventData);
	
	// Data
	///////

	std::vector<fw::T_EntityId> m_SelectedEntities;

	std::vector<I_SceneSelectionListener*> m_Listeners;

	vec4 m_OutlineColor = vec4(0.5f, 0.5f, 1.f, 1.f);

	EntityIdRenderer m_IdRenderer;
	bool m_IsIdRendererInitialized = false;

	Ptr<render::OutlineExtension> m_OutlineExtension;
};


} // namespace edit
} // namespace et
