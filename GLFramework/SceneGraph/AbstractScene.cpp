#include "stdafx.hpp"
#include "AbstractScene.hpp"

#include "../Components/CameraComponent.hpp"
#include "../Components/LightComponent.hpp"
#include "../Prefabs\FreeCamera.hpp"
#include "../Base/Time.hpp"
#include "Entity.hpp"
#include "../Framebuffers\Gbuffer.hpp"
#include "../Prefabs/Skybox.hpp"
#include "../Prefabs/FreeCamera.hpp"
#include "../Framebuffers/PostProcessingRenderer.hpp"
#include "../GraphicsHelper/TextRenderer.h"

#define CONTEXT Context::GetInstance()

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

	SafeDelete(m_pConObj);
	SafeDelete(m_pTime);
	SafeDelete(m_pGBuffer);
	SafeDelete(m_pDemoBuffer);
	SafeDelete(m_pPostProcessing);
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

	m_pPostProcessing = new PostProcessingRenderer();
	m_pPostProcessing->SetGamma(2.2f);
	m_pPostProcessing->SetExposure(1);
	m_pPostProcessing->SetBloomMultiplier(0.1f);
	m_pPostProcessing->SetBloomThreshold(10.0f);
	m_pPostProcessing->Initialize();

	m_pDemoBuffer = new Gbuffer(true);
	m_pDemoBuffer->Initialize();

	m_pGBuffer = new Gbuffer();
	m_pGBuffer->Initialize();
	m_pGBuffer->Enable(true);

	m_ClearColor = vec3(101.f / 255.f, 114.f / 255.f, 107.f / 255.f)*0.1f;

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

	PERFORMANCE->StartFrameTimer();

	m_pConObj->pCamera->Update();

	Update();
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_UP))
	{
		float newExp = m_Exposure * 4.f;
		m_Exposure += (newExp - m_Exposure)*TIME->DeltaTime();
		LOGGER::Log("Exposure: " + to_string(m_Exposure));
		m_pPostProcessing->SetExposure(m_Exposure);
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_DOWN))
	{
		float newExp = m_Exposure * 4.f;
		m_Exposure -= (newExp - m_Exposure)*TIME->DeltaTime();
		LOGGER::Log("Exposure: " + to_string(m_Exposure));
		m_pPostProcessing->SetExposure(m_Exposure);
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_LEFT) && m_UseSkyBox)
	{
		float r = min(max(m_pSkybox->GetRoughness() -TIME->DeltaTime(), 0.f), 1.f);
		LOGGER::Log("Roughness: " + to_string(r));
		m_pSkybox->SetRoughness(r);
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_RIGHT) && m_UseSkyBox)
	{
		float r = min(max(m_pSkybox->GetRoughness() + TIME->DeltaTime(), 0.f), 1.f);
		LOGGER::Log("Roughness: " + to_string(r));
		m_pSkybox->SetRoughness(r);
	}
	if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_RETURN))
	{
		m_DemoMode = !m_DemoMode;
	}
	if (m_DemoMode)
	{
		m_pPostProcessing->SetExposure(1);
		m_pPostProcessing->SetGamma(1);
	}
	else
	{
		m_pPostProcessing->SetExposure(m_Exposure);
		m_pPostProcessing->SetGamma(2.2f);
	}

	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootUpdate();
	}
	if (m_UseSkyBox)
	{
		m_pSkybox->RootUpdate();
	}
}

void AbstractScene::DrawShadow()
{
	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootDrawShadow();
	}
}

void AbstractScene::RootDraw()
{
	//Shadow Mapping
	//**************
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);//Maybe draw two sided materials in seperate pass
	auto lightVec = SCENE->GetLights(); //Todo: automatically add all light components to an array for faster access
	for (auto Light : lightVec)
	{
		Light->GenerateShadow();
	}

	//Deferred Rendering
	//******************
	//Step one: Draw the data onto gBuffer
	if (m_DemoMode)m_pDemoBuffer->Enable();
	else m_pGBuffer->Enable();

	//reset viewport
	int width = SETTINGS->Window.Width, height = SETTINGS->Window.Height;
	glViewport(0, 0, width, height);

	glClearColor(m_ClearColor.x, m_ClearColor.y, m_ClearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glCullFace(GL_BACK);
	Draw();
	for (Entity* pEntity : m_pEntityVec)
	{
		pEntity->RootDraw();
	}
	glDisable(GL_CULL_FACE);
	//Step two: blend data and calculate lighting with gbuffer
	m_pPostProcessing->EnableInput();
	if (m_DemoMode)m_pDemoBuffer->Draw();
	else
	{
		//Ambient IBL lighting
		m_pGBuffer->Draw();

		//copy Z-Buffer from gBuffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, m_pGBuffer->Get());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pPostProcessing->GetTargetFBO());
		glBlitFramebuffer(
			0, 0, SETTINGS->Window.Width, SETTINGS->Window.Height, 
			0, 0, SETTINGS->Window.Width, SETTINGS->Window.Height,
			GL_DEPTH_BUFFER_BIT, GL_NEAREST);

		//Render Light Volumes
		//glEnable(GL_STENCIL_TEST); //todo: lightvolume stencil test

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_ONE, GL_ONE);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		for (auto Light : lightVec)
		{
			Light->DrawVolume();
		}
		glCullFace(GL_BACK);
		glDisable(GL_BLEND);

		glEnable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);

		//glDisable(GL_STENCIL_TEST);

		m_pPostProcessing->EnableInput();

		//Foreward Rendering
		//******************
		//Step two: render with forward materials
		DrawForward();
		for (Entity* pEntity : m_pEntityVec)
		{
			pEntity->RootDrawForward();
		}
		if (m_UseSkyBox)
		{
			m_pSkybox->RootDrawForward();
		}
	}
	//m_pHDRbuffer->Draw();
	//Draw to default buffer
	m_pPostProcessing->Draw(0);

	TextRenderer::GetInstance()->Draw();

	PostDraw();
	
	PERFORMANCE->Update();
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

void AbstractScene::SetSkybox(string assetFile)
{
	m_UseSkyBox = true;
	SafeDelete(m_pSkybox);
	m_pSkybox = new Skybox(assetFile);
	m_pSkybox->RootInitialize();
}

HDRMap* AbstractScene::GetEnvironmentMap()
{
	if (m_UseSkyBox)
	{
		return m_pSkybox->GetHDRMap();
	}
	return nullptr;
}