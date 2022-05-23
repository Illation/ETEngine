#pragma once
#include <EtCore/Util/GenericEventDispatcher.h>


namespace et {
namespace fw {


class UnifiedScene;


//---------------------------
// E_SceneEvent
//
// List of events systems can listen for
//
typedef uint8 T_SceneEventFlags;
enum E_SceneEvent : T_SceneEventFlags
{
	Invalid = 0,

	SceneSwitch			= 1 << 0,
	Deactivated			= 1 << 1,
	Activated			= 1 << 2,
	RegisterSystems		= 1 << 3,
	PreLoadScreenGUI	= 1 << 4,

	All = 0xFF
};

//---------------------------
// SceneEventData
//
// Base scene event data contains only event type, but can be derived from to provide additional data
//
struct SceneEventData
{
public:
	SceneEventData(UnifiedScene* const uniScene) : scene(uniScene) {}
	virtual ~SceneEventData() = default;

	UnifiedScene* scene = nullptr;
};

//---------------------------
// SceneEventGUIData
//
// Scene event for PreLoadScreenGUI
//
struct SceneEventGUIData : public SceneEventData
{
public:
	SceneEventGUIData(UnifiedScene* const uniScene, core::HashString const docId, bool const loading)
		: SceneEventData(uniScene), 
		guiDocumentId(docId), 
		isLoading(loading) 
	{}
	virtual ~SceneEventGUIData() = default;

	core::HashString const guiDocumentId;
	bool const isLoading; // when false the document is unloading
};


typedef core::GenericEventDispatcher<T_SceneEventFlags, SceneEventData> T_SceneEventDispatcher;


typedef T_SceneEventDispatcher::T_CallbackId T_SceneEventCallbackId;
typedef T_SceneEventDispatcher::T_CallbackFn T_SceneEventCallback;


} // namespace fw
} // namespace et
