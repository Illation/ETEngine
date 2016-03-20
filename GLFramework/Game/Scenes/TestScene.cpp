#include "stdafx.hpp"

#include "TestScene.hpp"

#include "../Objects\Cube.hpp"
#include "../Materials/GbufferMaterial.hpp"
#include "../../Graphics\FrameBuffer.hpp"
#include "../../Framebuffers\Gbuffer.hpp"
#include "../../Framebuffers\HDRframeBuffer.hpp"

#include "../../Components/ModelComponent.hpp"

TestScene::TestScene() : AbstractScene("TestScene")
{
}
TestScene::~TestScene()
{
	SafeDelete(m_pMat);
	SafeDelete(m_pOutlineBuffer);
}

void TestScene::Initialize()
{
	m_pMat = new GbufferMaterial();
	m_pMat->SetDiffuseTexture("Resources/Textures/BaseColor.png");
	m_pMat->SetSpecularTexture("Resources/Textures/Roughness.png");
	m_pMat->SetNormalTexture("Resources/Textures/NormalMap.png");
	m_pMat->SetDifCol(glm::vec3(1.0f, 1.0f, 1.0f)*0.99f);

	unsigned amountPerRow = 20;
	float distance = 3.5f;
	float start = (amountPerRow / 2)*(-distance);
	for (size_t i = 0; i < amountPerRow; i++)
	{
		for (size_t j = 0; j < amountPerRow; j++)
		{
			auto pModelComp = new ModelComponent("Resources/Models/helmet.dae");
			pModelComp->SetMaterial(m_pMat);
			auto pSphere = new Entity();
			pSphere->AddComponent(pModelComp);
			pSphere->GetTransform()->Translate(vec3(start+i*distance, 0, start+j*distance));
			AddEntity(pSphere);
		}
	}

	m_pOutlineBuffer = new FrameBuffer("Resources/Shaders/outlineEffect.glsl", GL_UNSIGNED_BYTE);
	m_pOutlineBuffer->Initialize();
	m_pOutlineBuffer->Enable(false);
}

void TestScene::Update()
{
	LOGGER::Log("FPS: " + to_string(TIME->FPS()));
}

void TestScene::Draw()
{
}

void TestScene::PostDraw()
{
	//m_pOutlineBuffer->Enable(false);
	//m_pOutlineBuffer->Draw();
}