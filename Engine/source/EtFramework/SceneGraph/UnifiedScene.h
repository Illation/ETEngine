#pragma once
#include "SceneEvents.h"
#include "SceneDescriptor.h"

#include <EtCore/UpdateCycle/Context.h>
#include <EtCore/UpdateCycle/Tickable.h>

#include <EtRendering/SceneStructure/RenderScene.h>

#include <EtFramework/Config/TickOrder.h>
#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/Physics/PhysicsWorld.h>


// fwd
namespace et { namespace gui {
	class GuiExtension;
} }


namespace et {
namespace fw {


//--------------------
// UnifiedScene
//
// Combines all scene subsystems (ecs, rendering, physics) into a single globally accessible class, which manages update, loading, unloading and events
//
class UnifiedScene : public core::I_Tickable
{
	// static access
	//---------------
public:
	static UnifiedScene& Instance();

	// construct destruct
	//--------------------
private:
	UnifiedScene() : I_Tickable(static_cast<uint32>(E_TickOrder::TICK_SceneManager)) {}
public:
	UnifiedScene(UnifiedScene const&) = delete;
	void operator=(UnifiedScene const&) = delete;

	void Init();
	void Deinit();

	// tickable interface
	//--------------------
protected:
	void OnTick() override;

	// functionality
	//---------------
public:
	void LoadScene(core::HashString const assetId);
	void UnloadScene();

	void SetActiveCamera(T_EntityId const cameraId);

	// accessors
	//-----------
	bool IsSceneLoaded() const { return m_IsSceneLoaded; }
	core::HashString GetSceneId() const { return m_CurrentScene; }
	EcsController& GetEcs() { return m_Scene; }

	T_EntityId GetActiveCamera() const { return m_ActiveCamera; }
	T_EntityId GetAudioListener() const { return m_AudioListener; }

	render::Scene& GetRenderScene() { return m_RenderScene; }
	gui::GuiExtension* GetGuiExtension() { return m_GuiExtension.Get(); }
	PhysicsWorld& GetPhysicsWorld() { return m_PhysicsWorld; }

	T_SceneEventDispatcher& GetEventDispatcher() { return m_EventDispatcher; }

	// utility
	//---------
private:
	void AddEntity(EntityDescriptor const& entDesc, T_EntityId const parent);
	void PostLoadEntity(EntityDescriptor const& entDesc, T_EntityId const parent);

	// Data
	///////

	core::HashString m_CurrentScene;
	bool m_IsSceneLoaded = false;

	EcsController m_Scene;

	core::BaseContext m_Context;
	T_EntityId m_ActiveCamera = INVALID_ENTITY_ID;
	T_EntityId m_AudioListener = INVALID_ENTITY_ID;

	render::Scene m_RenderScene;

	Ptr<gui::GuiExtension> m_GuiExtension;

	PhysicsWorld m_PhysicsWorld;

	T_SceneEventDispatcher m_EventDispatcher;
};


} // namespace fw
} // namespace et
