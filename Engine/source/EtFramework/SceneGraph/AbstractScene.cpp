#include "stdafx.h"
#include "AbstractScene.h"

#include "Entity.h"

#include <EtCore/Helper/Time.h>

#include <EtRendering/SceneRendering/Gbuffer.h>
#include <EtRendering/SceneRendering/PostProcessingRenderer.h>
#include <EtRendering/SceneRendering/TextRenderer.h>

#include <EtFramework/Components/CameraComponent.h>
#include <EtFramework/Components/LightComponent.h>
#include <EtFramework/Physics/PhysicsWorld.h>
#include <EtFramework/SceneGraph/SceneManager.h>


//================
// Abstract Scene
//================


// construct destruct
//////////////////////

//----------------------
// AbstractScene::c-tor
//
AbstractScene::AbstractScene(std::string const& name) 
	: m_Name(name)
{ }

//----------------------
// AbstractScene::d-tor
//
AbstractScene::~AbstractScene()
{
	if (m_IsInitialized)
	{
		RootDeinit();
	}
}


// root
////////

//-----------------------------
// AbstractScene::RootActivate
//
void AbstractScene::RootActivate()
{
	RootInit();
	ContextManager::GetInstance()->SetActiveContext(m_SceneContext);
	OnActivated();
}

//-------------------------------
// AbstractScene::RootDeactivate
//
void AbstractScene::RootDeactivate()
{
	RootDeinit();
	OnDeactivated();
}

//-------------------------
// AbstractScene::RootInit
//
void AbstractScene::RootInit()
{
	if (m_IsInitialized)
	{
		return;
	}

	//Create SceneContext
	CameraComponent* const camComp = new CameraComponent();
	Entity* const camEntity = new Entity();
	camEntity->AddComponent(camComp);
	AddEntity(camEntity);

	m_SceneContext = new SceneContext();
	m_SceneContext->camera = camComp;
	m_SceneContext->scene = this;

	ContextManager::GetInstance()->SetActiveContext(m_SceneContext);

	m_pPhysicsWorld = new PhysicsWorld();
	m_pPhysicsWorld->Initialize();
	Init();

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootInit();
	}

	m_IsInitialized = true;

	m_SceneContext->time->Start();
}

//---------------------------
// AbstractScene::RootDeinit
//
void AbstractScene::RootDeinit()
{
	if (!m_IsInitialized)
	{
		return;
	}

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootDeinit();
	}

	for (Entity* pEntity : m_pEntityVec)
	{
		SafeDelete(pEntity);
	}

	m_pEntityVec.clear();

	ContextManager::GetInstance()->SetActiveContext(nullptr);
	SceneManager::GetInstance()->GetRenderScene().SetSkyboxMap(0u);
	SceneManager::GetInstance()->GetRenderScene().SetStarfield(0u);
	SceneManager::GetInstance()->GetRenderScene().SetPostProcessingSettings(PostProcessingSettings());

	SafeDelete(m_pPhysicsWorld);
	SafeDelete(m_SceneContext);

	m_IsInitialized = false;
}

//-----------------------------
// AbstractScene::RootUpdate
//
void AbstractScene::RootUpdate()
{
	Update();

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootUpdate();
	}

	m_pPhysicsWorld->Update();
}


// functionality
/////////////////

//-----------------------------
// AbstractScene::AddEntity
//
void AbstractScene::AddEntity(Entity* entity)
{
	entity->m_ParentScene = this;

	if (entity->GetId() == 0u)
	{
		std::string uniqueName;
		GetUniqueEntityName("Entity", uniqueName);
		entity->SetName(uniqueName);
	}

	entity->RootInit();

	m_pEntityVec.push_back(entity);
}

//-----------------------------
// AbstractScene::RemoveEntity
//
void AbstractScene::RemoveEntity(Entity* pEntity, bool deleteEntity)
{
	auto it = find(m_pEntityVec.begin(), m_pEntityVec.end(), pEntity);
	m_pEntityVec.erase(it);
	if (deleteEntity)
	{
		delete pEntity;
		pEntity = nullptr;
	}
	else pEntity->m_ParentScene = nullptr;
}

//--------------------------------
// AbstractScene::SetActiveCamera
//
void AbstractScene::SetActiveCamera(CameraComponent* pCamera)
{
	m_SceneContext->camera = pCamera;
}

//--------------------------
// AbstractScene::SetSkybox
//
void AbstractScene::SetSkybox(T_Hash const assetId)
{
	SceneManager::GetInstance()->GetRenderScene().SetSkyboxMap(assetId);
}

//-----------------------------
// AbstractScene::SetStarfield
//
void AbstractScene::SetStarfield(T_Hash const assetId)
{
	SceneManager::GetInstance()->GetRenderScene().SetStarfield(assetId);
}


// accessors
/////////////

//--------------------------
// AbstractScene::GetEntity
//
Entity* AbstractScene::GetEntity(T_Hash const id) const
{
	std::vector<Entity*> allEntities;
	for (Entity* const entity : m_pEntityVec)
	{
		entity->RecursiveAppendChildren(allEntities);
	}

	auto const foundIt = std::find_if(allEntities.begin(), allEntities.end(), [id](Entity const* const entity)
		{
			return entity->GetId() == id;
		});

	if (foundIt == allEntities.cend())
	{
		return nullptr;
	}

	return *foundIt;
}

//--------------------------------
// AbstractScene::GetActiveCamera
//
CameraComponent const* AbstractScene::GetActiveCamera() const
{
	ET_ASSERT(m_SceneContext != nullptr);
	return m_SceneContext->camera;
}


// utility
///////////

//------------------------------------
// AbstractScene::GetUniqueEntityName
//
void AbstractScene::GetUniqueEntityName(std::string const& suggestion, std::string& uniqueName) const
{
	uniqueName = suggestion;
	size_t index = 0;

	std::vector<Entity const*> allEntities;
	for (Entity const* const entity : m_pEntityVec)
	{
		entity->RecursiveAppendChildren(allEntities);
	}

	auto constructEntityName = [&]()
	{
		return suggestion + "_" + std::to_string(index);
	};

	while (!(std::find_if(allEntities.begin(), allEntities.end(), [&](Entity const* const entity)
	{
		return entity->GetName() == constructEntityName();
	}) == allEntities.end()))
	{
		index++;
	}

	uniqueName = constructEntityName();
}