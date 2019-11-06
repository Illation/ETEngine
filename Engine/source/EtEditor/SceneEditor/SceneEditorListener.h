#pragma once


//-------------------------------
// I_SceneEditorListener
//
// interface for a class that listens for events in the scene editor
//  - #todo: should probably eventually be replaced with an optional message systemq - i.e: SceneEventDispatcher
//
class I_SceneEditorListener
{
public:
	virtual ~I_SceneEditorListener() = default;

	virtual void OnShown() = 0;
	virtual void OnSceneSet() = 0;
	virtual void OnEditorTick() = 0;
};

