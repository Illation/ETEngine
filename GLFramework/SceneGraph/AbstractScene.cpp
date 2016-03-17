#include "stdafx.hpp"
#include "AbstractScene.hpp"

#include "../Components/CameraComponent.hpp"
#include "../Prefabs\FreeCamera.hpp"
#include "../Base/Time.hpp"
#include "Entity.hpp"

#define CONTEXT Context::GetInstance()

AbstractScene::AbstractScene(std::string name) : m_Name(name),
												m_IsInitialized(false)
{
}

AbstractScene::~AbstractScene()
{
	for (Entity* pEntity : m_pEntityVec)
	{
		delete pEntity;
		pEntity = nullptr;
	}
	m_pEntityVec.clear();
	delete m_pConObj;
	m_pConObj = nullptr;
	delete m_pTime;
	m_pTime = nullptr;
}

void AbstractScene::AddEntity(Entity* pEntity)
{
	pEntity->RootInitialize();
	pEntity->m_pParentScene = this;
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

	//Create DefaultCamera
	FreeCamera* freeCam = new FreeCamera();
	freeCam->GetTransform()->Translate(0, -1, -3.5);
	freeCam->GetTransform()->RotateEuler(glm::radians(20.f), 0, 0);
	AddEntity(freeCam);
	m_pDefaultCam = freeCam->GetComponent<CameraComponent>();
	m_pTime = new Time();
	m_pConObj = new ContextObjects();
	m_pConObj->pCamera = m_pDefaultCam;
	m_pConObj->pTime = m_pTime;

	CONTEXT->SetContext(m_pConObj);

	Initialize();

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootInitialize();
	}

	m_IsInitialized = true;

	m_pConObj->pTime->Start();
}

void AbstractScene::RootUpdate()
{
	m_pConObj->pTime->Update();
	m_pConObj->pCamera->Update();

	Update();

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootUpdate();
	}
}

void AbstractScene::RootDraw()
{
	Draw();

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootDraw();
	}

	PostDraw();
}

void AbstractScene::RootOnActivated()
{
	CONTEXT->SetContext(m_pConObj);
	OnActivated();
}
void AbstractScene::RootOnDeactivated()
{
	OnDeactivated();
}

void AbstractScene::SetActiveCamera(CameraComponent* pCamera)
{
	m_pConObj->pCamera = pCamera;
}