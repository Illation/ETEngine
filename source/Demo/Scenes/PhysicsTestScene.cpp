#include "stdafx.h"
#include "PhysicsTestScene.h"

#include <btBulletDynamicsCommon.h>

#include <Demo/Materials/TexPBRMaterial.h>
#include <Demo/Materials/EmissiveMaterial.h>

#include <EtCore/Content/ResourceManager.h>

#include <Engine/SceneGraph/Entity.h>
#include <Engine/Graphics/SpriteFont.h>
#include <Engine/Graphics/Light.h>
#include <Engine/Graphics/TextureData.h>
#include <Engine/GraphicsHelper/SpriteRenderer.h>
#include <Engine/GraphicsHelper/TextRenderer.h>
#include <Engine/GraphicsHelper/DebugRenderer.h>
#include <Engine/Helper/ScreenshotCapture.h>
#include <Engine/Components/ModelComponent.h>
#include <Engine/Components/LightComponent.h>
#include <Engine/Components/RigidBodyComponent.h>
#include <Engine/Components/AudioListenerComponent.h>
#include <Engine/Components/AudioSourceComponent.h>
#include <Engine/Physics/PhysicsManager.h>
#include <Engine/Physics/BulletETM.h>
#include <Engine/Physics/PhysicsWorld.h>
#include <Engine/Audio/AudioManager.h>
#include <Engine/Audio/AudioData.h>


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
	m_DebugFont = ResourceManager::GetInstance()->GetAssetData<SpriteFont>("Ubuntu-Regular.ttf"_hash);

	//Materials
	//**************************
	m_pFloorMat = new TexPBRMaterial(
		"mahogany_baseCol.png"_hash,
		"mahogany_rough.png"_hash,
		"bamboo_metal.png"_hash,
		"mahogany_ao.png"_hash,
		"mahogany_norm.png"_hash);
	m_pFloorMat->SetSpecular(0.5f);
	m_pBallMat = new TexPBRMaterial(
		"greasyMetal_baseCol.png"_hash,
		"greasyMetal_rough.png"_hash,
		"greasyMetal_metal.png"_hash,
		"mahogany_ao.png"_hash,
		"greasyMetal_norm.png"_hash);
	m_pBallMat->SetSpecular(0.5f);
	m_pBlockMat = new TexPBRMaterial(
		"bamboo_baseCol.png"_hash,
		"bamboo_rough.png"_hash,
		"bamboo_metal.png"_hash,
		"bamboo_ao.png"_hash,
		"bamboo_norm.png"_hash);
	m_pBlockMat->SetSpecular(0.5f);

	m_pLightMat = new EmissiveMaterial(vec3(500));

	GetPhysicsWorld()->GetWorld()->setGravity(ToBtVec3(vec3(0, -9.81f, 0)*0.1f));
	m_pSphereShape = PhysicsManager::GetInstance()->CreateSphereShape(m_SphereSize);

	SetSkybox("Ice_Lake_Ref.hdr"_hash);

	//Models
	//*************************
	//Floor
	{
		auto pModelComp = new ModelComponent("cube.dae"_hash);
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
			auto pModelComp = new ModelComponent("cube.dae"_hash);
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
	auto pModelComp1 = new ModelComponent("sphere.dae"_hash);
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
	m_AudioIdPlaylist = { "Disfigure-Blank.ogg"_hash,
		"testmusic.ogg"_hash,
		"pcm0844m.wav"_hash,
		"pcm0844s.wav"_hash,
		"pcm1644m.wav"_hash,
		"pcm1644s.wav"_hash };

	AudioManager::GetInstance()->SetDistanceModel(AL_INVERSE_DISTANCE);

	auto pListener = new AudioListenerComponent();
	CAMERA->GetEntity()->AddComponent(pListener);

	m_Source = new AudioSourceComponent();

	m_Source->SetAudioData(ResourceManager::GetInstance()->GetAssetData<AudioData>(m_AudioIdPlaylist[m_CurrentTrack]));
	m_Source->SetLooping(true);
	m_Source->Play();
	m_pLightEntity->AddComponent(m_Source);
}

void PhysicsTestScene::Update()
{
	vec3 lightPos = m_LightCentralPos + vec3(sin(TIME->GetTime()), 0.f, cos(TIME->GetTime()))*m_LightRotDistance;
	m_pLightEntity->GetTransform()->SetPosition(lightPos);

	if(INPUT->GetKeyState(E_KbdKey::Num_0) == E_KeyState::Pressed)
	{
		ScreenshotCapture::GetInstance()->Take();
	}

	if (INPUT->GetKeyState(E_KbdKey::LeftBracket) == E_KeyState::Pressed)
	{
		ET_ASSERT(m_AudioIdPlaylist.size() > 0u);
		if (m_CurrentTrack == 0u)
		{
			m_CurrentTrack = m_AudioIdPlaylist.size() - 1;
		}
		else
		{
			m_CurrentTrack--;
		}
		m_Source->SetAudioData(ResourceManager::GetInstance()->GetAssetData<AudioData>(m_AudioIdPlaylist[m_CurrentTrack]));
	}
	else if (INPUT->GetKeyState(E_KbdKey::RightBracket) == E_KeyState::Pressed)
	{
		ET_ASSERT(m_AudioIdPlaylist.size() > 0u);
		if (m_CurrentTrack == m_AudioIdPlaylist.size() - 1)
		{
			m_CurrentTrack = 0u;
		}
		else
		{
			m_CurrentTrack++;
		}
		m_Source->SetAudioData(ResourceManager::GetInstance()->GetAssetData<AudioData>(m_AudioIdPlaylist[m_CurrentTrack]));
	}

	if(INPUT->GetMouseButton(E_MouseButton::Right) == E_KeyState::Pressed)
	{
		auto pModelComp = new ModelComponent("sphere.dae"_hash);
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

	if(INPUT->GetKeyState(E_KbdKey::X) == E_KeyState::Pressed)
	{
		GRAPHICS.UseFXAA = !(GRAPHICS.UseFXAA);
	}

	SpriteRenderer::GetInstance()->Draw(m_DebugFont->GetAtlas(), vec2(1000, 0),
		vec4(1), vec2(0), vec2(1), 0, 0, SpriteRenderer::E_ScalingMode::TextureAbs);
}

void PhysicsTestScene::Draw()
{
	TextRenderer::GetInstance()->SetFont(m_DebugFont.get());
	TextRenderer::GetInstance()->SetColor(vec4(1, 0.3f, 0.3f, 1));
	std::string outString = "FPS: " + std::to_string(PERFORMANCE->GetRegularFPS());
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 20 + (m_DebugFont->GetFontSize()*1.1f) * 1));
	TextRenderer::GetInstance()->SetColor(vec4(1, 1, 1, 1));
	outString = "Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS());
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 20 + (m_DebugFont->GetFontSize()*1.1f) * 2));
	outString = "Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls);
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*1.1f) * 3), 128);
	outString = "VAWAVMVoV.";
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*1.1f) * 5), 128);

	outString = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do ";
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*2.5f) * 5), 64);
	outString = "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut ";
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*2.5f) * 6), 64);
	outString = "enim ad minim veniam, quis nostrud exercitation ullamco ";
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*2.5f) * 7), 64);
	outString = "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure ";
	TextRenderer::GetInstance()->DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*2.5f) * 8), 64);

	vec3 lightPos = m_pLightEntity->GetTransform()->GetPosition();
	DebugRenderer::GetInstance()->DrawLine(lightPos, lightPos + vec3(2, 0, 0), vec4(1, 0, 0, 1), 2);
	DebugRenderer::GetInstance()->DrawLine(lightPos, lightPos + vec3(0, 2, 0), vec4(0, 1, 0, 1), 2);
	DebugRenderer::GetInstance()->DrawLine(lightPos, lightPos + vec3(0, 0, 2), vec4(0, 0, 1, 1), 2);
}
