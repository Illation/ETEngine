#include "stdafx.h"
#include "PhysicsTestScene.h"

#include <btBulletDynamicsCommon.h>

#include <Runtime/Materials/TexPBRMaterial.h>
#include <Runtime/Materials/EmissiveMaterial.h>

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/SpriteFont.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtFramework/Components/ModelComponent.h>
#include <EtFramework/Components/LightComponent.h>
#include <EtFramework/Components/RigidBodyComponent.h>
#include <EtFramework/Components/AudioListenerComponent.h>
#include <EtFramework/Components/AudioSourceComponent.h>
#include <EtFramework/SceneGraph/Entity.h>
#include <EtFramework/Physics/PhysicsManager.h>
#include <EtFramework/Physics/BulletETM.h>
#include <EtFramework/Physics/PhysicsWorld.h>
#include <EtFramework/Audio/AudioManager.h>
#include <EtFramework/Audio/AudioData.h>


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

void PhysicsTestScene::Init()
{
	//Fonts
	//**************************
	m_DebugFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Ubuntu-Regular.ttf"_hash);

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

	// Moving point light
	auto lightEntity = new Entity();

	auto lightMeshEntity = new Entity();
	lightEntity->AddChild(lightMeshEntity);

	auto pModelComp1 = new ModelComponent("sphere.dae"_hash);
	pModelComp1->SetMaterial(m_pLightMat);
	lightMeshEntity->GetTransform()->Scale(vec3(0.01f));
	lightMeshEntity->AddComponent(pModelComp1);

	m_Light = new LightComponent(LightComponent::Type::Point, vec3(1, 1, 1), 900.f);
	lightEntity->AddComponent(m_Light);
	lightEntity->GetTransform()->Scale(vec3(10.f));
	lightEntity->GetTransform()->SetPosition(vec3(0, 50, 0));
	AddEntity(lightEntity);

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

	m_Source->SetAudioData(ResourceManager::Instance()->GetAssetData<AudioData>(m_AudioIdPlaylist[m_CurrentTrack]));
	m_Source->SetLooping(true);
	m_Source->Play();
	lightEntity->AddComponent(m_Source);
}

void PhysicsTestScene::Update()
{
	vec3 lightPos = m_LightCentralPos + vec3(sin(TIME->GetTime()), 0.f, cos(TIME->GetTime()))*m_LightRotDistance;
	m_Light->GetTransform()->SetPosition(lightPos);

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
		m_Source->SetAudioData(ResourceManager::Instance()->GetAssetData<AudioData>(m_AudioIdPlaylist[m_CurrentTrack]));
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
		m_Source->SetAudioData(ResourceManager::Instance()->GetAssetData<AudioData>(m_AudioIdPlaylist[m_CurrentTrack]));
	}

	if(INPUT->GetMouseButton(E_MouseButton::Right) >= E_KeyState::Down)
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

	render::ShadedSceneRenderer::GetCurrent()->GetSpriteRenderer().Draw(m_DebugFont->GetAtlas(), vec2(1000, 0),
		vec4(1), vec2(0), vec2(1), 0, 0, SpriteRenderer::E_ScalingMode::TextureAbs);

	TextRenderer& textRenderer = render::ShadedSceneRenderer::GetCurrent()->GetTextRenderer();

	textRenderer.SetFont(m_DebugFont.get());
	textRenderer.SetColor(vec4(1, 0.3f, 0.3f, 1));
	std::string outString = "FPS: " + std::to_string(PERFORMANCE->GetRegularFPS());
	textRenderer.DrawText(outString, vec2(20, 20 + (m_DebugFont->GetFontSize()*1.1f) * 1));
	textRenderer.SetColor(vec4(1, 1, 1, 1));
	outString = "Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS());
	textRenderer.DrawText(outString, vec2(20, 20 + (m_DebugFont->GetFontSize()*1.1f) * 2));
	outString = "Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls);
	textRenderer.DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*1.1f) * 3), 128);
	outString = "VAWAVMVoV.";
	textRenderer.DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*1.1f) * 5), 128);

	outString = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do ";
	textRenderer.DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*2.5f) * 5), 64);
	outString = "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut ";
	textRenderer.DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*2.5f) * 6), 64);
	outString = "enim ad minim veniam, quis nostrud exercitation ullamco ";
	textRenderer.DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*2.5f) * 7), 64);
	outString = "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure ";
	textRenderer.DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*2.5f) * 8), 64);

	//vec3 lightPos = m_Light->GetTransform()->GetPosition();
	//render::ShadedSceneRenderer::GetCurrent()->GetDebugRenderer().DrawLine(lightPos, lightPos + vec3(2, 0, 0), vec4(1, 0, 0, 1), 2);
	//render::ShadedSceneRenderer::GetCurrent()->GetDebugRenderer().DrawLine(lightPos, lightPos + vec3(0, 2, 0), vec4(0, 1, 0, 1), 2);
	//render::ShadedSceneRenderer::GetCurrent()->GetDebugRenderer().DrawLine(lightPos, lightPos + vec3(0, 0, 2), vec4(0, 0, 1, 1), 2);
}
