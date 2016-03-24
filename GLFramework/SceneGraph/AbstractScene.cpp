#include "stdafx.hpp"
#include "AbstractScene.hpp"

#include "../Components/CameraComponent.hpp"
#include "../Components/LightComponent.hpp"
#include "../Prefabs\FreeCamera.hpp"
#include "../Base/Time.hpp"
#include "Entity.hpp"
#include "../Framebuffers\Gbuffer.hpp"
#include "../Framebuffers\HDRframeBuffer.hpp"

#define CONTEXT Context::GetInstance()

AbstractScene::AbstractScene(std::string name) : m_Name(name),
												m_IsInitialized(false)
{
}

AbstractScene::~AbstractScene()
{
	for (Entity* pEntity : m_pEntityVec)
	{
		SafeDelete(pEntity);
	}
	m_pEntityVec.clear();

	SafeDelete(m_pConObj);
	SafeDelete(m_pTime);
	SafeDelete(m_pGBuffer);
	SafeDelete(m_pDemoBuffer);
	SafeDelete(m_pHDRbuffer);
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

	//Create SceneContext
	FreeCamera* freeCam = new FreeCamera();
	freeCam->GetTransform()->Translate(0, -1, -3.5);
	freeCam->GetTransform()->RotateEuler(glm::radians(20.f), 0, 0);
	AddEntity(freeCam);
	m_pDefaultCam = freeCam->GetComponent<CameraComponent>();
	m_pTime = new Time();
	m_pConObj = new ContextObjects();
	m_pConObj->pCamera = m_pDefaultCam;
	m_pConObj->pTime = m_pTime;
	m_pConObj->pScene = this;

	CONTEXT->SetContext(m_pConObj);

	m_ClearColor = vec3(101.f / 255.f, 114.f / 255.f, 107.f / 255.f)*0.1f;

	Initialize();

	m_pHDRbuffer = new HDRframeBuffer();
	m_pHDRbuffer->SetGamma(2.2f);
	m_pHDRbuffer->Initialize();

	m_pDemoBuffer = new Gbuffer(true);
	m_pDemoBuffer->SetAmbCol(glm::vec3(0.05f, 0.1f, 0.08f)*0.01f);
	m_pDemoBuffer->Initialize();

	m_pGBuffer = new Gbuffer();
	m_pGBuffer->SetAmbCol(glm::vec3(0.05f, 0.1f, 0.08f)*0.0f);
	m_pGBuffer->Initialize();
	m_pGBuffer->Enable(true);


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
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_UP))
	{
		m_Exposure *= 1.02f;
		LOGGER::Log("Exposure: " + to_string(m_Exposure));
		m_pHDRbuffer->SetExposure(m_Exposure);
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_DOWN))
	{
		m_Exposure /= 1.02f;
		LOGGER::Log("Exposure: " + to_string(m_Exposure));
		m_pHDRbuffer->SetExposure(m_Exposure);
	}
	if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_RETURN))
	{
		m_DemoMode = !m_DemoMode;
	}
	if (m_DemoMode)
	{
		m_pHDRbuffer->SetExposure(1);
		m_pHDRbuffer->SetGamma(1);
	}
	else
	{
		m_pHDRbuffer->SetExposure(m_Exposure);
		m_pHDRbuffer->SetGamma(2.2f);
	}

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootUpdate();
	}
}

void AbstractScene::RootDraw()
{
	//Deferred Rendering
	//******************
	//Step one: Draw the data onto gBuffer
	if (m_DemoMode)m_pDemoBuffer->Enable();
	else m_pGBuffer->Enable();

	glClearColor(m_ClearColor.x, m_ClearColor.y, m_ClearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Draw();
	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootDraw();
	}
	//Step two: blend data and calculate lighting with gbuffer
	m_pHDRbuffer->Enable();
	if (m_DemoMode)m_pDemoBuffer->Draw();
	else
	{
		m_pGBuffer->Draw();

		//Foreward Rendering
		//******************
		//Step one: copy Z-Buffer from gBuffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_pGBuffer->Get());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pHDRbuffer->Get());
		glBlitFramebuffer(
			0, 0, SETTINGS->Window.Width, SETTINGS->Window.Height, 0, 0,
			SETTINGS->Window.Width, SETTINGS->Window.Height,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		m_pHDRbuffer->Enable();
		//Step two: render with forward materials
		DrawForward();
		for (Entity* pEntity : m_pEntityVec)
		{
			pEntity->RootDrawForward();
		}
	}
	m_pHDRbuffer->Enable(false);
	m_pHDRbuffer->Draw();

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

std::vector<LightComponent*> AbstractScene::GetLights()
{
	vector<LightComponent*> ret;
	for (auto *pEntity : m_pEntityVec)
	{
		auto entityLights = pEntity->GetComponents<LightComponent>();
		ret.insert(ret.end(), entityLights.begin(), entityLights.end());
	}
	return ret;
}