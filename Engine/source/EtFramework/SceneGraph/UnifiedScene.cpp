#include "stdafx.h"
#include "UnifiedScene.h"

#include <btBulletDynamicsCommon.h>

#include <EtCore/Helper/Context.h>
#include <EtCore/Content/ResourceManager.h>

#include <EtFramework/Physics/BulletETM.h>
#include <EtFramework/Systems/TransformSystem.h>
#include <EtFramework/Systems/LightSystem.h>
#include <EtFramework/Systems/ModelInit.h>


namespace framework {


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
	m_Scene.RegisterSystem<TransformSystem>();
	m_Scene.RegisterOnComponentAdded(
		std::function<void(framework::EcsController&, TransformComponent&, T_EntityId const)>(TransformSystem::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(
		std::function<void(framework::EcsController&, TransformComponent&, T_EntityId const)>(TransformSystem::OnComponentRemoved));

	m_Scene.RegisterSystem<LightSystem>();
	m_Scene.RegisterOnComponentAdded(std::function<void(framework::EcsController&, LightComponent&, T_EntityId const)>(LightSystem::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(std::function<void(framework::EcsController&, LightComponent&, T_EntityId const)>(LightSystem::OnComponentRemoved));

	m_Scene.RegisterOnComponentAdded(std::function<void(framework::EcsController&, ModelComponent&, T_EntityId const)>(ModelInit::OnComponentAdded));
	m_Scene.RegisterOnComponentRemoved(std::function<void(framework::EcsController&, ModelComponent&, T_EntityId const)>(ModelInit::OnComponentRemoved));

	m_EventDispatcher.Notify(E_SceneEvent::RegisterSystems, new SceneEventData(nullptr));
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

	// notification before beginning the process so systems can prepare (spash screen, loading bar, timer etc)
	m_EventDispatcher.Notify(E_SceneEvent::SceneSwitch, new SceneEventData(nullptr));

	if (m_CurrentScene != 0u)
	{
		UnloadScene();
	}

	// timer etc
	ContextManager::GetInstance()->SetActiveContext(&m_Context);

	m_PhysicsWorld.Initialize();

	m_CurrentScene = assetId;

	// load scene descriptor and translate into ecs
	//----------------------------------------------
	AssetPtr<SceneDescriptor> const sceneDesc = ResourceManager::Instance()->GetAssetData<SceneDescriptor>(m_CurrentScene);
	ET_ASSERT(sceneDesc != nullptr);

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

	m_RenderScene.SetPostProcessingSettings(sceneDesc->postprocessing);

	// audio settings
	m_AudioListener = sceneDesc->audioListener.id;

	// physics settings
	m_PhysicsWorld.GetWorld()->setGravity(ToBtVec3(sceneDesc->gravity));

	// done loading
	//--------------
	m_EventDispatcher.Notify(E_SceneEvent::Activated, new SceneEventData(nullptr));
	m_Context.time->Start();
}

//---------------------------
// UnifiedScene::UnloadScene
//
void UnifiedScene::UnloadScene()
{
	// notification first
	m_EventDispatcher.Notify(E_SceneEvent::Deactivated, new SceneEventData(nullptr));

	// clear
	m_Scene.RemoveAllEntities();

	// reset rendering
	m_RenderScene.SetSkyboxMap(0u);
	m_RenderScene.SetStarfield(0u);
	m_RenderScene.SetPostProcessingSettings(PostProcessingSettings());

	// reset physics
	m_PhysicsWorld.Deinit();

	// reset time
	ContextManager::GetInstance()->SetActiveContext(nullptr);

	// invalidate state
	m_ActiveCamera = INVALID_ENTITY_ID;
	m_AudioListener = INVALID_ENTITY_ID;

	m_CurrentScene = 0u;
}

//-------------------------
// UnifiedScene::AddEntity
//
// Add an entity to the ecs and resolve dependency links
//  - set parent to INVALID_ENTITY_ID to indicate a root entity
//
void UnifiedScene::AddEntity(EntityDescriptor const& entDesc, T_EntityId const parent)
{
	T_EntityId const id = m_Scene.AddEntityChild(parent);

	// update all entity links referring to the assigned ID
	EntityLinkResolver::Instance().OnEntityIdAssigned(entDesc.GetId(), id);

	// create a component data list
	std::vector<RawComponentPtr> components;
	for (I_ComponentDescriptor* const compDesc : entDesc.GetComponents())
	{
		components.push_back(compDesc->MakeRawData());
	}

	// add the components to our new entity
	// we copy the pointer list because the pointers are repointed to the constructed components, which doesn't allow us to free the original memory
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


} // namespace framework
