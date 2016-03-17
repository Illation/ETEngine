#include "stdafx.hpp"

#include "TestScene.hpp"

#include "../Objects\Cube.hpp"
#include "../Materials/UberMaterial.hpp"
#include "../../Graphics\FrameBuffer.hpp"
#include "../../Framebuffers\HDRframeBuffer.hpp"

#include "../../Components/ModelComponent.hpp"

TestScene::TestScene() : AbstractScene("TestScene")
{
}
TestScene::~TestScene()
{
	delete m_pMat;
	m_pMat = nullptr;
	delete m_pOutlineBuffer;
	delete m_pHDRbuffer;
}

void TestScene::Initialize()
{
	m_ClearColor = vec3(101.f / 255.f, 114.f / 255.f, 107.f / 255.f)*0.1f;

	m_pMat = new UberMaterial();
	//m_pMat->SetDiffuseTexture("Resources/Textures/BaseColor.png");
	m_pMat->SetNormalTexture("Resources/Textures/NormalMap.png");
	m_pMat->SetAmbCol(glm::vec3(0.05f, 0.1f, 0.08f)*0.1f);
	m_pMat->SetDifCol(glm::vec3(1.0f, 1.0f, 1.0f)*0.99f);
	m_pMat->SetLightDir(glm::normalize(glm::vec3(0.5, 1, 0.5)));
	auto pModelComp = new ModelComponent("Resources/Models/helmet.dae");
	pModelComp->SetMaterial(m_pMat);
	auto pSphere = new Entity();
	pSphere->AddComponent(pModelComp);
	AddEntity(pSphere);

	m_pOutlineBuffer = new FrameBuffer("Resources/Shaders/outlineEffect.glsl", GL_UNSIGNED_BYTE);
	m_pOutlineBuffer->Initialize();
	m_pOutlineBuffer->Enable(false);

	m_pHDRbuffer = new HDRframeBuffer();
	m_pHDRbuffer->Initialize();
	m_pHDRbuffer->Enable(true);
}

void TestScene::Update()
{
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_UP))
	{
		m_Exposure *= 1.02f;
		cout << m_Exposure << endl;
	}
	if (INPUT->IsKeyboardKeyDown(SDL_SCANCODE_DOWN))
	{
		m_Exposure /= 1.02f;
		cout << m_Exposure << endl;
	}
	m_pHDRbuffer->SetExposure(m_Exposure);
}

void TestScene::Draw()
{
	m_pHDRbuffer->Enable();
	// Clear the screen to white
	glClearColor(m_ClearColor.x, m_ClearColor.y, m_ClearColor.z, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//std::cout << TIME->FPS() << std::endl;
}

void TestScene::PostDraw()
{
	// Bind default framebuffer and draw contents of our framebuffer
	m_pHDRbuffer->Enable(false);
	m_pHDRbuffer->Draw();
	//m_pOutlineBuffer->Enable(false);
	//m_pOutlineBuffer->Draw();
}