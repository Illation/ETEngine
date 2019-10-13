#pragma once
#include "EditorBase.h"

#include "SceneSelection.h"
#include "SceneEditorListener.h"


//---------------------------------
// SceneEditor
//
// Editor that modifies a scene. It currently can't make permanent changes as scenes are still code defined instead of data driven
//
class SceneEditor final : public EditorBase
{
public:
	// definitions
	//--------------
	static std::string const s_EditorName;

	// construct destruct
	//--------------------
	SceneEditor() : EditorBase() {}
	~SceneEditor();

	// EditorBase interface
	//--------------------
	void Init(Gtk::Frame* const parent) override;
	std::string const& GetName() const override { return s_EditorName; }

	// accessors
	//-----------
	SceneSelection& GetSceneSelection() { return m_SceneSelection; }

	// functionality
	//----------------
	void RegisterListener(I_SceneEditorListener* const listener);
	void UnregisterListener(I_SceneEditorListener const* const listener);

private:

	// Data
	///////

	SceneSelection m_SceneSelection;

	std::vector<I_SceneEditorListener*> m_Listeners;
};

