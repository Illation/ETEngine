#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class Cube;
class FrameBuffer;
class UberMaterial;

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
	UberMaterial* m_pMat = nullptr;
	FrameBuffer* m_pPostProc = nullptr;
};

