#include "stdafx.h"
#include "UnifiedScene.h"

#include <btBulletDynamicsCommon.h>

#include <EtCore/Util/Context.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtFramework/Physics/BulletETM.h>
#include <EtFramework/Systems/TransformSystem.h>
#include <EtFramework/Systems/RigidBodySystem.h>
#include <EtFramework/Systems/LightSystem.h>
#include <EtFramework/Systems/ModelInit.h>
#include <EtFramework/Systems/PlanetInit.h>
#include <EtFramework/Systems/AtmosphereInit.h>
#include <EtFramework/Systems/AudioListenerSystem.h>
#include <EtFramework/Systems/AudioSourceSystem.h>
#include <EtFramework/Systems/PlanetCameraClippingSystem.h>
#include <EtFramework/Components/SpriteComponent.h>


namespace et {
namespace fw {


//===============
// Unified Scene 
//===============
	

//------------------------
// UnifiedScene::Instance
//
// Global singleton access
//
UnifiedScene& UnifiedScene::Instance()
{
	static UnifiedScene instance;
	return instance;
}

//--------------------
// UnifiedScene::Init
//
// Register systems here
//
void UnifiedScene::Init()
{
	// component init / deinint
	m_Scene.RegisterOnComponentAdded(T_CompEventFn<TransformComponent>(TransformSystem::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(T_CompEventFn<TransformComponent>(TransformSystem::OnComponentRemoved));

	m_Scene.RegisterOnComponentAdded(T_CompEventFn<LightComponent>(LightSystem::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(T_CompEventFn<LightComponent>(LightSystem::OnComponentRemoved));

	m_Scene.RegisterOnComponentAdded(T_CompEventFn<ModelComponent>(ModelInit::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(T_CompEventFn<ModelComponent>(ModelInit::OnComponentRemoved));

	m_Scene.RegisterOnComponentAdded(T_CompEventFn<PlanetComponent>(PlanetInit::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(T_CompEventFn<PlanetComponent>(PlanetInit::OnComponentRemoved));

	m_Scene.RegisterOnComponentAdded(T_CompEventFn<AtmosphereComponent>(AtmosphereInit::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(T_CompEventFn<AtmosphereComponent>(AtmosphereInit::OnComponentRemoved));

	m_Scene.RegisterOnComponentAdded(T_CompEventFn<RigidBodyComponent>(RigidBodySystem::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(T_CompEventFn<RigidBodyComponent>(RigidBodySystem::OnComponentRemoved));

	m_Scene.RegisterOnComponentAdded(T_CompEventFn<SpriteComponent>(SpriteComponent::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(T_CompEventFn<SpriteComponent>(SpriteComponent::OnComponentRemoved));

	m_Scene.RegisterOnComponentAdded(T_CompEventFn<AudioSourceComponent>(AudioSourceSystem::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(T_CompEventFn<AudioSourceComponent>(AudioSourceSystem::OnComponentRemoved));

	// systems - listed in roughly the order they execute in
	m_Scene.RegisterSystem<RigidBodySystem>();
	m_Scene.RegisterSystem<TransformSystem::Compute>();
	m_Scene.RegisterSystem<AudioSourceSystem::Translate>();
	m_Scene.RegisterSystem<TransformSystem::Reset>();
	m_Scene.RegisterSystem<AudioSourceSystem::State>();
	m_Scene.RegisterSystem<AudioListenerSystem>();
	m_Scene.RegisterSystem<PlanetCameraLinkSystem>();
	m_Scene.RegisterSystem<LightSystem>();

	// allow users of the framework to also register for events
	m_EventDispatcher.Notify(E_SceneEvent::RegisterSystems, new SceneEventData(this));
}

//----------------------
// UnifiedScene::OnTick
//
void UnifiedScene::OnTick()
{
	if (m_CurrentScene != 0u)
	{
		m_Scene.Process();
		m_PhysicsWorld.Update();

		// update camera in render scene
	}
}

//-------------------------
// UnifiedScene::LoadScene
//
void UnifiedScene::LoadScene(T_Hash const assetId)
{
	// preparation
	//-------------

	// notification before beginning the process so systems can prepare (splash screen, loading bar, timer etc)
	m_EventDispatcher.Notify(E_SceneEvent::SceneSwitch, new SceneEventData(this));

	if (m_CurrentScene != 0u)
	{
		UnloadScene();
	}

	// timer etc
	core::ContextManager::GetInstance()->SetActiveContext(&m_Context);

	m_PhysicsWorld.Initialize();

	m_CurrentScene = assetId;

	// load scene descriptor and translate into ECS
	//----------------------------------------------
	AssetPtr<SceneDescriptor> const sceneDesc = core::ResourceManager::Instance()->GetAssetData<SceneDescriptor>(m_CurrentScene);
	ET_ASSERT(sceneDesc != nullptr);

	m_SceneName = sceneDesc.GetAsset()->GetName();

	for (EntityDescriptor const& entDesc : sceneDesc->entities)
	{
		AddEntity(entDesc, INVALID_ENTITY_ID);
	}

	// while adding entities all entity links should be resolved, so we can now reset the link resolver
	EntityLinkResolver::Instance().Clear();

	// render settings
	if (!(sceneDesc->skybox.empty()))
	{
		m_RenderScene.SetSkyboxMap(GetHash(sceneDesc->skybox));
	}

	if (!(sceneDesc->starfield.empty()))
	{
		m_RenderScene.SetStarfield(GetHash(sceneDesc->starfield));
	}

	m_ActiveCamera = sceneDesc->activeCamera.id;
	ET_ASSERT(m_ActiveCamera != INVALID_ENTITY_ID);

	m_RenderScene.SetPostProcessingSettings(sceneDesc->postprocessing);

	// audio settings
	m_AudioListener = sceneDesc->audioListener.id;
	if (m_AudioListener != INVALID_ENTITY_ID)
	{
		ET_ASSERT(m_Scene.HasComponent<AudioListenerComponent>(m_AudioListener));
		m_Scene.AddComponents(m_AudioListener, ActiveAudioListenerComponent());
	}

	// physics settings
	m_PhysicsWorld.GetWorld()->setGravity(ToBtVec3(sceneDesc->gravity));

	// post load for components that need it
	for (EntityDescriptor const& entDesc : sceneDesc->entities)
	{
		PostLoadEntity(entDesc, INVALID_ENTITY_ID);
	}

	// done loading
	//--------------
	m_EventDispatcher.Notify(E_SceneEvent::Activated, new SceneEventData(this));
	m_Context.time->Start();
}

//---------------------------
// UnifiedScene::UnloadScene
//
void UnifiedScene::UnloadScene()
{
	// notification first
	m_EventDispatcher.Notify(E_SceneEvent::Deactivated, new SceneEventData(this));

	// clear
	m_Scene.RemoveAllEntities();

	// reset rendering
	m_RenderScene.SetSkyboxMap(0u);
	m_RenderScene.SetStarfield(0u);
	m_RenderScene.SetPostProcessingSettings(render::PostProcessingSettings());

	// reset physics
	m_PhysicsWorld.Deinit();

	// reset time
	core::ContextManager::GetInstance()->SetActiveContext(nullptr);

	// invalidate state
	m_ActiveCamera = INVALID_ENTITY_ID;
	m_AudioListener = INVALID_ENTITY_ID;

	m_CurrentScene = 0u;
	m_SceneName = "";
}

//-------------------------
// UnifiedScene::AddEntity
//
// Add an entity to the ECS and resolve dependency links
//  - set parent to INVALID_ENTITY_ID to indicate a root entity
//
void UnifiedScene::AddEntity(EntityDescriptor const& entDesc, T_EntityId const parent)
{
	T_EntityId const id = m_Scene.AddEntityChild(parent);

	// update all entity links referring to the assigned ID
	EntityLinkResolver::Instance().OnEntityIdAssigned(entDesc.GetId(), id);
	entDesc.SetAssignedId(id);

	// create a component data list
	std::vector<RawComponentPtr> components;
	for (I_ComponentDescriptor* const compDesc : entDesc.GetComponents())
	{
		components.push_back(compDesc->MakeRawData());
	}

	// add the components to our new entity
	// we copy the pointer list because the pointers are "repointed" to the constructed components, which doesn't allow us to free the original memory
	m_Scene.AddComponents(id, std::vector<RawComponentPtr>(components)); 

	// we can now free the memory of our raw component data - this needs to be done manually as we are using void*
	for (RawComponentPtr& comp : components)
	{
		ComponentRegistry::Instance().GetFullDestructor(comp.typeIdx)(comp.data);
	}

	components.clear();

	// add the child entities
	for (EntityDescriptor const& childDesc : entDesc.GetChildren())
	{
		AddEntity(childDesc, id);
	}
}

//------------------------------
// UnifiedScene::PostLoadEntity
//
// Allow components to be post load initialized once all entities and components are added, useful for linking component data together
//
void UnifiedScene::PostLoadEntity(EntityDescriptor const& entDesc, T_EntityId const parent)
{
	T_EntityId const id = entDesc.GetAssignedId();

	for (I_ComponentDescriptor* const compDesc : entDesc.GetComponents())
	{
		if (compDesc->CallScenePostLoad())
		{
			void* const rawComp = m_Scene.GetComponentData(id, compDesc->GetType());
			compDesc->OnScenePostLoadRoot(m_Scene, id, rawComp);
		}
	}

	// recurse for children
	for (EntityDescriptor const& childDesc : entDesc.GetChildren())
	{
		PostLoadEntity(childDesc, id);
	}
}


} // namespace fw
} // namespace et
