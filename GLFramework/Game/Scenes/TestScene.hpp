#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class Cube;
class FrameBuffer;
class DiffuseMaterial;

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
	DiffuseMaterial* m_pDifMat = nullptr;
	FrameBuffer* m_pPostProc = nullptr;
};

