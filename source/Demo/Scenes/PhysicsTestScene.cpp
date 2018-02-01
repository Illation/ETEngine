#include "stdafx.hpp"
#include "PhysicsTestScene.h"
#include "..\Engine\GraphicsHelper\TextRenderer.hpp"
#include "..\Engine\SceneGraph\Entity.hpp"
#include "..\Engine\Components\ModelComponent.hpp"
#include "../../Engine/Components/LightComponent.hpp"
#include "../../Engine/Graphics/SpriteFont.hpp"
#include "../../Engine/Graphics/Light.hpp"
#include "TexPBRMaterial.hpp"
#include "EmissiveMaterial.hpp"
#include "..\Engine\Physics\PhysicsManager.h"
#include "..\Engine\Components\RigidBodyComponent.h"

#include <btBulletDynamicsCommon.h>
#include "..\Engine\Physics\BulletETM.h"
#include "..\Engine\SceneGraph\AbstractScene.hpp"
#include "..\Engine\Physics\PhysicsWorld.h"

PhysicsTestScene::PhysicsTestScene() : AbstractScene("PhysicsTestScene")
{
}
PhysicsTestScene::~PhysicsTestScene()
{
	SafeDelete(m_pFloorMat);
	SafeDelete(m_pBallMat);
	SafeDelete(m_pBlockMat);
	SafeDelete(m_pLightMat);
}

void PhysicsTestScene::Initialize()
{
	//Fonts
	//**************************
	m_pDebugFont = ContentManager::Load<SpriteFont>("Resources/Fonts/Consolas_32.fnt");

	//Materials
	//**************************
	m_pFloorMat = new TexPBRMaterial(
		"Resources/Textures/Mahogany/baseCol.png",
		"Resources/Textures/Mahogany/rough.png",
		"Resources/Textures/Bamboo/metal.png",
		"Resources/Textures/Mahogany/ao.png",
		"Resources/Textures/Mahogany/norm.png");
	m_pFloorMat->SetSpecular(0.5f);
	m_pBallMat = new TexPBRMaterial(
		"Resources/Textures/GreasyMetal/baseCol.png",
		"Resources/Textures/GreasyMetal/rough.png",
		"Resources/Textures/GreasyMetal/metal.png",
		"Resources/Textures/Mahogany/ao.png",
		"Resources/Textures/GreasyMetal/norm.png");
	m_pBallMat->SetSpecular(0.5f);
	m_pBlockMat = new TexPBRMaterial(
		"Resources/Textures/Bamboo/baseCol.png",
		"Resources/Textures/Bamboo/rough.png",
		"Resources/Textures/Bamboo/metal.png",
		"Resources/Textures/Bamboo/ao.png",
		"Resources/Textures/Bamboo/norm.png");
	m_pBlockMat->SetSpecular(0.5f);

	m_pLightMat = new EmissiveMaterial(vec3(500));

	GetPhysicsWorld()->GetWorld()->setGravity(ToBtVec3(vec3(0, -9.81f, 0)*0.1f));
	m_pSphereShape = PhysicsManager::GetInstance()->CreateSphereShape(m_SphereSize);

	SetSkybox("Resources/Textures/Ice_Lake_Ref.hdr");
	//Models
	//*************************
	//Floor
	{
		auto pModelComp = new ModelComponent("Resources/Models/cube.dae");
		pModelComp->SetMaterial(m_pFloorMat);
		auto pFloor = new Entity();
		pFloor->AddComponent(pModelComp);
		pFloor->GetTransform()->SetPosition(vec3(0));
		pFloor->GetTransform()->Scale(1000, 1, 1000);

		btBoxShape *pFloorBoxShape = PhysicsManager::GetInstance()->CreateBoxShape(vec3(1000, 1, 1000)*0.01f);
		auto pFloorRBComp = new RigidBodyComponent(true);
		pFloorRBComp->SetCollisionShape(pFloorBoxShape);
		pFloor->AddComponent(pFloorRBComp);

		AddEntity(pFloor);
	}

	btBoxShape *pBlockShape = PhysicsManager::GetInstance()->CreateBoxShape(vec3(9.85f, 6.f, 30.f)*0.01f);

	uint32 amountPerRow = 3;
	uint32 levels = 50;
	float distance = 0.2f;
	float start = (amountPerRow / 2)*(-distance);
	float baseHeight = 0.076f;
	for (uint32 level = 0; level < levels; level++)
	{
		for (size_t i = 0; i < amountPerRow; i++)
		{
			auto pModelComp = new ModelComponent("Resources/Models/cube.dae");
			pModelComp->SetMaterial(m_pBlockMat);
			auto pBlock = new Entity();
			pBlock->AddComponent(pModelComp);
			if (level % 2 < 1)
			{
				pBlock->GetTransform()->SetPosition(vec3(start + i*distance, baseHeight +(level*0.122f), 0));
			}
			else
			{
				pBlock->GetTransform()->SetPosition(vec3(0, baseHeight +(level*0.122f), start + i*distance));
				pBlock->GetTransform()->Rotate(quat(vec3(0, 1, 0), etm::PI_DIV2));
			}
			pBlock->GetTransform()->Scale(vec3(9.85f, 6.f, 30.f));

			auto pRBComp = new RigidBodyComponent();
			pRBComp->SetCollisionShape(pBlockShape);
			pRBComp->SetMass(m_BlockMass);
			pBlock->AddComponent(pRBComp);

			AddEntity(pBlock);
		}
	}
	//Lights
	//**************************

	//Directional
	auto pModelComp1 = new ModelComponent("Resources/Models/sphere.dae");
	pModelComp1->SetMaterial(m_pLightMat);
	auto pLigEntity = new Entity();
	pLigEntity->AddComponent(pModelComp1);
	auto pLight = new DirectionalLight(vec3(1, 1, 1), 10.f);
	pLight->SetShadowEnabled(true);
	pLigEntity->AddComponent(new LightComponent(pLight));
	pLigEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	pLigEntity->GetTransform()->Rotate(quat(vec3(1, 0, 1), -etm::PI_DIV4));
	pLigEntity->GetTransform()->SetPosition(vec3(0, 50, 0));
	AddEntity(pLigEntity);

	//Camera
	//**************************
	CAMERA->GetTransform()->Translate(vec3(0, 2, 0));

	SETTINGS->Window.VSync(false);
}

void PhysicsTestScene::Update()
{
	if (INPUT->IsMouseButtonPressed(SDL_BUTTON_RIGHT))
	{
		auto pModelComp = new ModelComponent("Resources/Models/sphere.dae");
		pModelComp->SetMaterial(m_pBallMat);
		auto pBall = new Entity();
		pBall->AddComponent(pModelComp);
		vec3 bDir = CAMERA->GetTransform()->GetForward();
		vec3 bPos = CAMERA->GetTransform()->GetPosition() + bDir*m_SphereSize;
		pBall->GetTransform()->SetPosition(bPos);
		pBall->GetTransform()->Scale(vec3(m_SphereSize));

		auto pRBComp = new RigidBodyComponent();
		pRBComp->SetCollisionShape(m_pSphereShape);
		pRBComp->SetMass(m_SphereMass);

		pBall->AddComponent(pRBComp);

		AddEntity(pBall);

		pRBComp->ApplyImpulse(bDir*m_SphereForce);
	}
}

void PhysicsTestScene::Draw()
{
	TextRenderer::GetInstance()->SetFont(m_pDebugFont);
	TextRenderer::GetInstance()->SetColor(vec4(1, 0.3f, 0.3f, 1));
	std::string outString = "FPS: " + std::to_string(PERFORMANCE->GetRegularFPS());
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 20));
	TextRenderer::GetInstance()->SetColor(vec4(1, 1, 1, 1));
	outString = "Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS());
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 50));
	outString = "Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls);
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 80));
}
