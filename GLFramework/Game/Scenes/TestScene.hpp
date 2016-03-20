#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class Cube;
class FrameBuffer;
class Gbuffer;
class HDRframeBuffer;
class GbufferMaterial;

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
	GbufferMaterial* m_pMat = nullptr;
	FrameBuffer* m_pOutlineBuffer = nullptr;
};

