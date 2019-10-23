#pragma once
#include "EditorBase.h"

#include "SceneSelection.h"
#include "SceneEditorListener.h"

#include <gdk/gdkevents.h>

#include <EtCore/UpdateCycle/Tickable.h>


class SceneViewport;


//---------------------------------
// SceneEditor
//
// Editor that modifies a scene. It currently can't make permanent changes as scenes are still code defined instead of data driven
//
class SceneEditor final : public EditorBase, public I_Tickable
{
	// definitions
	//--------------
public:
	static std::string const s_EditorName;
	static std::string const s_LayoutName;

	// construct destruct
	//--------------------
	SceneEditor() : EditorBase(), I_Tickable(static_cast<uint32>(E_EditorTickOrder::TICK_SceneEditor)) {}
	~SceneEditor();

	// EditorBase interface
	//--------------------
	void InitInternal() override;
	std::string const& GetName() const override { return s_EditorName; }
	std::string const& GetLayoutName() const override { return s_LayoutName; }

	bool OnKeyEvent(bool const pressed, GdkEventKey* const evnt) override;

	// I_Tickable interface
	//----------------------
private:
	void OnTick() override;

	// accessors
	//-----------
public:
	SceneSelection& GetSceneSelection() { return m_SceneSelection; }

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
};

