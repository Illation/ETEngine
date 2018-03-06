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
#include "..\Engine\Components\AudioListenerComponent.h"
#include "..\Engine\Components\AudioSourceComponent.h"
#include "..\Engine\Audio\AudioManager.h"
#include "..\Engine\Content\AudioLoader.h"
#include "..\Engine\GraphicsHelper\DebugRenderer.h"
#include "..\Engine\Helper\ScreenshotCapture.h"
#include "..\Engine\GraphicsHelper\SpriteRenderer.hpp"

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
	m_pDebugFont = ContentManager::Load<SpriteFont>("Resources/Fonts/Ubuntu-Regular.ttf");
	//m_pDebugFont = ContentManager::Load<SpriteFont>("Resources/Fonts/Consolas_32.fnt");

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
	m_pLightEntity = new Entity();
	m_pLightEntity->AddComponent(pModelComp1);
	auto pLight = new DirectionalLight(vec3(1, 1, 1), 10.f);
	pLight->SetShadowEnabled(true);
	m_pLightEntity->AddComponent(new LightComponent(pLight));
	m_pLightEntity->GetTransform()->Scale(0.1f, 0.1f, 0.1f);
	m_pLightEntity->GetTransform()->Rotate(quat(vec3(1, 0, 1), -etm::PI_DIV4));
	m_pLightEntity->GetTransform()->SetPosition(vec3(0, 50, 0));
	AddEntity(m_pLightEntity);

	//Camera
	//**************************
	CAMERA->GetTransform()->Translate(vec3(0, 2, 0));
	m_LightCentralPos = CAMERA->GetTransform()->GetPosition();

	//Audio
	//**************************
	AudioManager::GetInstance()->SetDistanceModel(AL_INVERSE_DISTANCE);

	auto pListener = new AudioListenerComponent();
	CAMERA->GetEntity()->AddComponent(pListener);

	auto pSource = new AudioSourceComponent();

	AudioLoader* pLoader = CONTENT::GetLoader<AudioLoader, AudioData>();
	pLoader->ForceMono(true);
	pSource->SetAudioData(CONTENT::Load<AudioData>("Resources/Sounds/Disfigure-Blank.ogg"));
	//pSource->SetAudioData(CONTENT::Load<AudioData>("Resources/Sounds/testmusic.ogg"));
	//pSource->SetAudioData(CONTENT::Load<AudioData>("Resources/Sounds/pcm0844m.wav"));
	//pSource->SetAudioData(CONTENT::Load<AudioData>("Resources/Sounds/pcm0844s.wav"));
	//pSource->SetAudioData(CONTENT::Load<AudioData>("Resources/Sounds/pcm1644m.wav"));
	//pSource->SetAudioData(CONTENT::Load<AudioData>("Resources/Sounds/pcm1644s.wav"));
	pLoader->ForceMono(false);
	pSource->SetLooping(true);
	pSource->Play();
	m_pLightEntity->AddComponent(pSource);

	//vec3 testVec = vec3(1, 2, 3);
	//LOG(testVec);
	//rttr::property prop = rttr::type::get(testVec).get_property("x");
	//rttr::variant vec_value = prop.get_value(testVec);
	//LOG(std::to_string(vec_value.get_value<float>()));
	//prop.set_value(testVec, 3);
	//LOG(testVec);

	SETTINGS->Window.VSync(false);
}

void PhysicsTestScene::Update()
{
	vec3 lightPos = m_LightCentralPos + vec3(sin(TIME->GetTime()), 0.f, cos(TIME->GetTime()))*m_LightRotDistance;
	m_pLightEntity->GetTransform()->SetPosition(lightPos);

	if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_0))
	{
		ScreenshotCapture::GetInstance()->Take();
	}

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

	if (INPUT->IsKeyboardKeyPressed('X'))
	{
		GRAPHICS.UseFXAA = !(GRAPHICS.UseFXAA);
	}

	SpriteRenderer::GetInstance()->Draw(m_pDebugFont->GetAtlas(), vec2(200, 200), 
		vec4(1), vec2(0), vec2(4), 0, 0, SpriteScalingMode::TEXTURE_ABS);
}

void PhysicsTestScene::Draw()
{
	TextRenderer::GetInstance()->SetFont(m_pDebugFont);
	TextRenderer::GetInstance()->SetColor(vec4(1, 0.3f, 0.3f, 1));
	std::string outString = "FPS: " + std::to_string(PERFORMANCE->GetRegularFPS());
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 40));
	TextRenderer::GetInstance()->SetColor(vec4(1, 1, 1, 1));
	outString = "Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS());
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 72));
	outString = "Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls);
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 104));

	vec3 lightPos = m_pLightEntity->GetTransform()->GetPosition();
	DebugRenderer::GetInstance()->DrawLine(lightPos, lightPos + vec3(2, 0, 0), vec4(1, 0, 0, 1), 2);
	DebugRenderer::GetInstance()->DrawLine(lightPos, lightPos + vec3(0, 2, 0), vec4(0, 1, 0, 1), 2);
	DebugRenderer::GetInstance()->DrawLine(lightPos, lightPos + vec3(0, 0, 2), vec4(0, 0, 1, 1), 2);
}
