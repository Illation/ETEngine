#pragma once
#include "EditorBase.h"

#include "SceneSelection.h"

#include <Engine/GraphicsContext/Viewport.h>

#include <EtEditor/UI/Outliner.h>


//---------------------------------
// SceneEditor
//
// Editor that modifies a scene. It currently can't make permanent changes as scenes are still code defined instead of data driven
//
class SceneEditor final : public I_Editor
{
public:
	// definitions
	//--------------
	static std::string const s_EditorName;

	// construct destruct
	//--------------------
	SceneEditor() : I_Editor() {}
	~SceneEditor();

	// I_Editor interface
	//--------------------
	void Init(Gtk::Frame* const parent) override;
	std::string const& GetName() const override { return s_EditorName; }

	// functionality
	// these should eventually be replaced by a generic tool creation interface
	//--------------------------------------------------------------------------
	std::unique_ptr<Viewport> CreateSceneViewport();
	std::unique_ptr<Outliner> CreateOutliner();

	// accessors
	//-----------
	static E_MouseButton GetButtonFromGtk(uint32 const buttonCode);
	static E_KbdKey GetKeyFromGtk(uint32 const keyCode);

	SceneSelection& GetSceneSelection() { return m_SceneSelection; }

private:

	// Data
	///////

	Glib::RefPtr<Gtk::Builder> m_RefBuilder;

	SceneSelection m_SceneSelection;

	bool m_IsNavigating = false;

	std::unique_ptr<Viewport> m_SceneViewport;
	std::unique_ptr<Outliner> m_Outliner;
};

