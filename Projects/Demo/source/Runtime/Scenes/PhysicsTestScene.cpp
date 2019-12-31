#include "stdafx.h"
#include "PhysicsTestScene.h"

#include <btBulletDynamicsCommon.h>

#include <EtCore/Content/ResourceManager.h>

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
#include <EtFramework/Audio/AudioData.h>


void PhysicsTestScene::Init()
{
	//Materials
	//**************************
	GetPhysicsWorld()->GetWorld()->setGravity(ToBtVec3(vec3(0, -9.81f, 0)*0.1f));
	m_pSphereShape = PhysicsManager::GetInstance()->CreateSphereShape(m_SphereSize);

	SetSkybox("Ice_Lake_Ref.hdr"_hash);

	//Models
	//*************************
	//Floor
	{
		auto pModelComp = new ModelComponent("cube.dae"_hash, "MI_TexPBR_Floor.json"_hash);
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
			auto pModelComp = new ModelComponent("cube.dae"_hash, "MI_TexPBR_Block.json"_hash);
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

	auto pModelComp1 = new ModelComponent("sphere.dae"_hash, "MI_Emissive_LightBulb.json"_hash);
	lightMeshEntity->GetTransform()->Scale(vec3(0.01f));
	lightMeshEntity->AddComponent(pModelComp1);

	m_Light = new LightComponent(LightComponent::Type::Point, vec3(1, 1, 1), 900.f);
	lightEntity->AddComponent(m_Light);
	lightEntity->GetTransform()->Scale(vec3(10.f));
	lightEntity->GetTransform()->SetPosition(vec3(0, 50, 0));
	AddEntity(lightEntity);

	m_LightCentralPos = vec3(0.f, 2.f, 0.f);

	//Audio
	//**************************
	m_AudioIdPlaylist = { "Disfigure-Blank.ogg"_hash,
		"testmusic.ogg"_hash,
		"pcm0844m.wav"_hash,
		"pcm0844s.wav"_hash,
		"pcm1644m.wav"_hash,
		"pcm1644s.wav"_hash };

	m_Source = new AudioSourceComponent();

	m_Source->SetAudioData(ResourceManager::Instance()->GetAssetData<AudioData>(m_AudioIdPlaylist[m_CurrentTrack]));
	m_Source->SetLooping(true);
	m_Source->Play();
	lightEntity->AddComponent(m_Source);

	//Camera (+ audio listener)
	//**************************
	auto pListener = new AudioListenerComponent();
	CAMERA->GetEntity()->AddComponent(pListener);
	CAMERA->GetTransform()->SetPosition(0, 2, -10);
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
		auto pModelComp = new ModelComponent("sphere.dae"_hash, "MI_TexPBR_Ball.json"_hash);
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
