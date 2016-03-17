#pragma once
#include "../../SceneGraph/AbstractScene.hpp"

class Cube;
class FrameBuffer;
class HDRframeBuffer;
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
	FrameBuffer* m_pOutlineBuffer = nullptr;
	HDRframeBuffer* m_pHDRbuffer = nullptr;

	float m_Exposure = 1;
	glm::vec3 m_ClearColor;
};

