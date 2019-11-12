#include "stdafx.h"
#include "AbstractScene.h"

#include "Entity.h"

#include <EtCore/Helper/Time.h>

#include <Engine/Components/CameraComponent.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/FreeCamera.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/Prefabs/FreeCamera.h>
#include <Engine/SceneRendering/Gbuffer.h>
#include <Engine/SceneRendering/PostProcessingRenderer.h>
#include <Engine/SceneRendering/TextRenderer.h>
#include <Engine/Physics/PhysicsWorld.h>
#include <Engine/SceneGraph/SceneManager.h>


AbstractScene::AbstractScene(std::string name) 
	: m_Name(name)
	, m_IsInitialized(false)
{
}

AbstractScene::~AbstractScene()
{
	SceneManager::GetInstance()->GetRenderScene().SetSkyboxMap(0u);
	for (Entity* pEntity : m_pEntityVec)
	{
		SafeDelete(pEntity);
	}
	m_pEntityVec.clear();
	if (m_pSkybox)SafeDelete(m_pSkybox);

	SafeDelete(m_pPhysicsWorld);
	SafeDelete(m_SceneContext);
}

void AbstractScene::AddEntity(Entity* entity)
{
	entity->m_pParentScene = this;

	if (entity->GetId() == 0u)
	{
		std::string uniqueName;
		GetUniqueEntityName("Entity", uniqueName);
		entity->SetName(uniqueName);
	}

	entity->RootInitialize();

	m_pEntityVec.push_back(entity);
}

void AbstractScene::RemoveEntity(Entity* pEntity, bool deleteEntity)
{
	auto it = find(m_pEntityVec.begin(), m_pEntityVec.end(), pEntity);
	m_pEntityVec.erase(it);
	if (deleteEntity)
	{
		delete pEntity;
		pEntity = nullptr;
	}
	else pEntity->m_pParentScene = nullptr;
}

void AbstractScene::RootInitialize()
{
	if (m_IsInitialized)return;

	//Create SceneContext
	FreeCamera* freeCam = new FreeCamera();
	freeCam->GetTransform()->SetPosition(0, -1, -3.5);
	freeCam->GetTransform()->RotateEuler(etm::radians(20.f), 0, 0);
	AddEntity(freeCam);

	m_SceneContext = new SceneContext();
	m_SceneContext->camera = freeCam->GetComponent<CameraComponent>();
	m_SceneContext->scene = this;

	ContextManager::GetInstance()->SetActiveContext(m_SceneContext);

	m_PostProcessingSettings = PostProcessingSettings();

	m_pPhysicsWorld = new PhysicsWorld();
	m_pPhysicsWorld->Initialize();

	Initialize();

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootInitialize();
	}

	m_IsInitialized = true;
	m_EventDispatcher.Notify(new SceneEventData(E_SceneEvent::Initialized));

	m_SceneContext->time->Start();
}

void AbstractScene::RootUpdate()
{
	// active time and performance are updated by tick manager
	//m_SceneContext->time->Update();

	//PERFORMANCE->StartFrameTimer();

	Update();
	if(INPUT->GetKeyState(E_KbdKey::Up) == E_KeyState::Down)
	{
		float exposure = m_PostProcessingSettings.exposure;
		float newExp = exposure * 4.f;
		exposure += (newExp - exposure)*TIME->DeltaTime();
		LOG("Exposure: " + std::to_string(exposure));
		m_PostProcessingSettings.exposure = exposure;
	}
	if(INPUT->GetKeyState(E_KbdKey::Down) == E_KeyState::Down)
	{
		float exposure = m_PostProcessingSettings.exposure;
		float newExp = exposure * 4.f;
		exposure -= (newExp - exposure)*TIME->DeltaTime();
		LOG("Exposure: " + std::to_string(exposure));
		m_PostProcessingSettings.exposure = exposure;
	}
	if (INPUT->GetKeyState(E_KbdKey::Left) == E_KeyState::Down && m_UseSkyBox)
	{
		float r = std::min(std::max(m_pSkybox->GetRoughness() -TIME->DeltaTime(), 0.f), 1.f);
		LOG("Roughness: " + std::to_string(r));
		SceneManager::GetInstance()->GetRenderScene().SetSkyboxRoughness(r);
	}
	if (INPUT->GetKeyState(E_KbdKey::Right) == E_KeyState::Down && m_UseSkyBox)
	{
		float r = std::min(std::max(m_pSkybox->GetRoughness() + TIME->DeltaTime(), 0.f), 1.f);
		LOG("Roughness: " + std::to_string(r));
		SceneManager::GetInstance()->GetRenderScene().SetSkyboxRoughness(r);
	}

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootUpdate();
	}
	if (m_UseSkyBox)
	{
		m_pSkybox->RootUpdate();
	}

	m_pPhysicsWorld->Update();
}

void AbstractScene::RootOnActivated()
{
	RootInitialize();
	ContextManager::GetInstance()->SetActiveContext(m_SceneContext);
	OnActivated();
}
void AbstractScene::RootOnDeactivated()
{
	OnDeactivated();
}

void AbstractScene::SetActiveCamera(CameraComponent* pCamera)
{
	m_SceneContext->camera = pCamera;
}

std::vector<LightComponent*> AbstractScene::GetLights()
{
	std::vector<LightComponent*> ret;
	for (auto *pEntity : m_pEntityVec)
	{
		auto entityLights = pEntity->GetComponents<LightComponent>();
		ret.insert(ret.end(), entityLights.begin(), entityLights.end());
	}
	return ret;
}

const PostProcessingSettings& AbstractScene::GetPostProcessingSettings() const
{
	//Any settings blending should be done here
	return m_PostProcessingSettings;
}

CameraComponent const* AbstractScene::GetActiveCamera() const
{
	ET_ASSERT(m_SceneContext != nullptr);
	return m_SceneContext->camera;
}

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

void AbstractScene::SetSkybox(T_Hash const assetId)
{
	SceneManager::GetInstance()->GetRenderScene().SetSkyboxMap(assetId);
}

EnvironmentMap const* AbstractScene::GetEnvironmentMap() const
{
	if (m_UseSkyBox)
	{
		return m_pSkybox->GetHDRMap();
	}
	return nullptr;
}

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