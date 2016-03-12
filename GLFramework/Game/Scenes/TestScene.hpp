#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class Camera;
class Cube;
class FrameBuffer;

class TestScene : public AbstractScene
{
public:
	TestScene();
	~TestScene();
private:
	void Initialize();
	void Update();
	void Draw();
	void PostDraw();
private:
	Camera* m_pMainCam = nullptr;
	Cube* m_pTestCube = nullptr;
	FrameBuffer* m_pPostProc = nullptr;
};

