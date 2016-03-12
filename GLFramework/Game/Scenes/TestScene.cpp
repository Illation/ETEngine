#include "TestScene.hpp"

#include "../Objects\Cube.hpp"
#include "../../Graphics\FrameBuffer.hpp"
#include "../../Graphics\Camera.hpp"

TestScene::TestScene() : AbstractScene("TestScene")
{
}
TestScene::~TestScene()
{
	delete m_pMainCam;
	delete m_pPostProc;
}

void TestScene::Initialize()
{
	m_pMainCam = new Camera();
	m_pMainCam->SetFieldOfView(45);
	m_pMainCam->SetNearClippingPlane(1);
	m_pMainCam->SetFarClippingPlane(10);
	m_pMainCam->UsePerspectiveProjection();
	SetActiveCamera(m_pMainCam);

	m_pTestCube = new Cube();
	AddEntity(m_pTestCube);

	m_pPostProc = new FrameBuffer();
	m_pPostProc->Initialize();
}

void TestScene::Update()
{

}

void TestScene::Draw()
{
	m_pPostProc->Enable();
	// Clear the screen to white
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void TestScene::PostDraw()
{
	// Bind default framebuffer and draw contents of our framebuffer
	m_pPostProc->Enable(false);
	m_pPostProc->Draw();
}