#include "TestScene.hpp"

#include "../Objects\Cube.hpp"
#include "../Materials/DiffuseMaterial.hpp"
#include "../../Graphics\FrameBuffer.hpp"

#include "../../Components/ModelComponent.hpp"

#include "../../Content/ContentManager.hpp"
#include "../../Base/Context.hpp"

#define TIME Context::GetInstance()->pTime

TestScene::TestScene() : AbstractScene("TestScene")
{
}
TestScene::~TestScene()
{
	delete m_pDifMat;
	m_pDifMat = nullptr;
	delete m_pPostProc;
}

void TestScene::Initialize()
{
	m_pDifMat = new DiffuseMaterial();
	m_pDifMat->SetAmbCol(glm::vec3(0.05f, 0.1f, 0.08f)*4.f);
	m_pDifMat->SetDifCol(glm::vec3(0.8f, 0.8f, 0.8f));
	m_pDifMat->SetLightDir(glm::normalize(glm::vec3(0.5, 1, 0.5)));
	auto pModelComp = new ModelComponent("Resources/monkey.dae");
	pModelComp->SetMaterial(m_pDifMat);
	auto pSphere = new Entity();
	pSphere->AddComponent(pModelComp);
	AddEntity(pSphere);

	m_pPostProc = new FrameBuffer();
	m_pPostProc->Initialize();
	m_pPostProc->Enable(false);
}

void TestScene::Update()
{

}

void TestScene::Draw()
{
	m_pPostProc->Enable();
	// Clear the screen to white
	glClearColor((float)(101.f / 255.f)*0.3f, (float)(114.f / 255.f)*0.3f, (float)(107.f / 255.f)*0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//std::cout << TIME->FPS() << std::endl;
}

void TestScene::PostDraw()
{
	// Bind default framebuffer and draw contents of our framebuffer
	m_pPostProc->Enable(false);
	m_pPostProc->Draw();
}