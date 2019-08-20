#include "stdafx.h"
#include "AbstractScene.h"

#include "Entity.h"

#include <EtCore/Helper/Time.h>

#include <Engine/Components/CameraComponent.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Prefabs/FreeCamera.h>
#include <Engine/Prefabs/Skybox.h>
#include <Engine/Prefabs/FreeCamera.h>
#include <Engine/Framebuffers/Gbuffer.h>
#include <Engine/Framebuffers/PostProcessingRenderer.h>
#include <Engine/GraphicsHelper/TextRenderer.h>
#include <Engine/GraphicsHelper/RenderPipeline.h>
#include <Engine/Physics/PhysicsWorld.h>


AbstractScene::AbstractScene(std::string name) 
	: m_Name(name)
	, m_IsInitialized(false)
{
}

AbstractScene::~AbstractScene()
{
	for (Entity* pEntity : m_pEntityVec)
	{
		SafeDelete(pEntity);
	}
	m_pEntityVec.clear();
	if (m_pSkybox)SafeDelete(m_pSkybox);

	SafeDelete(m_pPhysicsWorld);
	SafeDelete(m_SceneContext);
}

void AbstractScene::AddEntity(Entity* pEntity)
{
	pEntity->m_pParentScene = this;
	pEntity->RootInitialize();
	m_pEntityVec.push_back(pEntity);
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

	m_SceneContext->time->Start();
}

void AbstractScene::RootUpdate()
{
	// active time and performance are updated by tick manager
	//m_SceneContext->time->Update();

	//PERFORMANCE->StartFrameTimer();

	m_SceneContext->camera->Update();

	Update();
	if(INPUT->GetKeyState(static_cast<uint32>(SDLK_UP)) == E_KeyState::Down)
	{
		float exposure = m_PostProcessingSettings.exposure;
		float newExp = exposure * 4.f;
		exposure += (newExp - exposure)*TIME->DeltaTime();
		LOG("Exposure: " + std::to_string(exposure));
		m_PostProcessingSettings.exposure = exposure;
	}
	if(INPUT->GetKeyState(static_cast<uint32>(SDLK_DOWN)) == E_KeyState::Down)
	{
		float exposure = m_PostProcessingSettings.exposure;
		float newExp = exposure * 4.f;
		exposure -= (newExp - exposure)*TIME->DeltaTime();
		LOG("Exposure: " + std::to_string(exposure));
		m_PostProcessingSettings.exposure = exposure;
	}
	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_LEFT)) == E_KeyState::Down && m_UseSkyBox)
	{
		float r = std::min(std::max(m_pSkybox->GetRoughness() -TIME->DeltaTime(), 0.f), 1.f);
		LOG("Roughness: " + std::to_string(r));
		m_pSkybox->SetRoughness(r);
	}
	if (INPUT->GetKeyState(static_cast<uint32>(SDLK_RIGHT)) == E_KeyState::Down && m_UseSkyBox)
	{
		float r = std::min(std::max(m_pSkybox->GetRoughness() + TIME->DeltaTime(), 0.f), 1.f);
		LOG("Roughness: " + std::to_string(r));
		m_pSkybox->SetRoughness(r);
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

void AbstractScene::SetSkybox(T_Hash const assetId)
{
	m_UseSkyBox = true;
	SafeDelete(m_pSkybox);
	m_pSkybox = new Skybox(assetId);
	m_pSkybox->RootInitialize();
	m_pSkybox->SetRoughness(0.15f);
}

EnvironmentMap const* AbstractScene::GetEnvironmentMap() const
{
	if (m_UseSkyBox)
	{
		return m_pSkybox->GetHDRMap();
	}
	return nullptr;
}