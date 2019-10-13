#pragma once
#include "EditorBase.h"

#include "SceneSelection.h"

#include <EtEditor/UI/Outliner.h>
#include <EtEditor/UI/SceneViewport.h>


// forward
namespace Gtk {
	class Paned;
}


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
	std::unique_ptr<SceneViewport> CreateSceneViewport();
	std::unique_ptr<Outliner> CreateOutliner();

	SceneSelection& GetSceneSelection() { return m_SceneSelection; }

	static Gtk::Frame* CreateInnerFrame(Gtk::Paned* const split, bool const isFirst);

private:

	// Data
	///////

	SceneSelection m_SceneSelection;

	bool m_IsNavigating = false;

	std::unique_ptr<SceneViewport> m_SceneViewport;
	std::unique_ptr<Outliner> m_Outliner;
};

