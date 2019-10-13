#pragma once


//-------------------------------
// I_SceneEditorListener
//
// interface for a class that listens for events in the scene editor
//
class I_SceneEditorListener
{
public:
	virtual ~I_SceneEditorListener() = default;

	virtual void OnShown() = 0;
	virtual void OnSceneSet() = 0;
};

