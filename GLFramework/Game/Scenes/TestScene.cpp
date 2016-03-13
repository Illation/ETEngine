#include "TestScene.hpp"

#include "../Objects\Cube.hpp"
#include "../../Graphics\FrameBuffer.hpp"

TestScene::TestScene() : AbstractScene("TestScene")
{
}
TestScene::~TestScene()
{
	delete m_pPostProc;
}

void TestScene::Initialize()
{
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