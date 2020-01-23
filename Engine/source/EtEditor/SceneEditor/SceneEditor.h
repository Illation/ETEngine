#pragma once
#include "SceneSelection.h"
#include "SceneEditorListener.h"

#include <gdk/gdk.h>

#include <EtCore/UpdateCycle/Tickable.h>

#include <EtFramework/ECS/ComponentRegistry.h>

#include <EtEditor/Layout/EditorBase.h>


namespace et {
namespace edit {


class SceneViewport;


//---------------------------------
// EditorMetaComponent
//
// Meta data for entities
//
struct EditorMetaComponent final
{
	ECS_DECLARE_COMPONENT
public:

	std::string name;
};


//---------------------------------
// SceneEditor
//
// Editor that modifies a scene. It currently can't make permanent changes as scenes are still code defined instead of data driven
//
class SceneEditor final : public EditorBase, public core::I_Tickable
{
	// definitions
	//--------------
public:
	static std::string const s_EditorName;
	static std::string const s_LayoutName;
	static std::vector<E_EditorTool> const s_SupportedTools;

	// static functionality
	//----------------------
	static void RegisterEcsEvents();

	// construct destruct
	//--------------------
	SceneEditor() : EditorBase(), core::I_Tickable(static_cast<uint32>(E_EditorTickOrder::TICK_SceneEditor)) {}
	~SceneEditor();

	// EditorBase interface
	//--------------------
	void InitInternal() override;
	std::string const& GetName() const override { return s_EditorName; }
	std::string const& GetLayoutName() const override { return s_LayoutName; }
	std::vector<E_EditorTool> const& GetSupportedTools() const override { return s_SupportedTools; }

	bool OnKeyEvent(bool const pressed, GdkEventKey* const evnt) override;

	// I_Tickable interface
	//----------------------
private:
	void OnTick() override;

	// accessors
	//-----------
public:
	SceneSelection& GetSceneSelection() { return m_SceneSelection; }
	bool IsShown() const { return m_IsShown; }

	// functionality
	//----------------
	void RegisterListener(I_SceneEditorListener* const listener);
	void UnregisterListener(I_SceneEditorListener const* const listener);

	void SetNavigatingViewport(SceneViewport* const sceneViewport) { m_NavigatingViewport = sceneViewport; }

private:

	// Data
	///////

	SceneSelection m_SceneSelection;

	SceneViewport* m_NavigatingViewport;
	std::vector<I_SceneEditorListener*> m_Listeners;

	bool m_IsShown = false;
};


} // namespace edit
} // namespace et
