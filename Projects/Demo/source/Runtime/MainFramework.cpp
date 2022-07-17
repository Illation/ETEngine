﻿#include "stdafx.h"
#include "MainFramework.h"

#include "SpawnSystem.h"
#include "LightControlSystem.h"
#include "SwirlyLightSystem.h"
#include "CelestialBodySystem.h"
#include "PlaylistSystem.h"
#include "FreeCamera.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/SceneGraph/SceneEvents.h>

#include <Common/CustomTestAsset.h>
#include <Common/DemoUI.h>


namespace et {
namespace demo {


//================
// Main Framework
//================


//-----------------------------
// MainFramework::OnSystemInit
//
void MainFramework::OnSystemInit()
{
	fw::EcsController& ecs = fw::UnifiedScene::Instance().GetEcs();

	ecs.RegisterSystem<FreeCameraSystem>();
	ecs.RegisterSystem<SpawnSystem>();
	ecs.RegisterSystem<LightControlSystem>();
	ecs.RegisterSystem<SwirlyLightSystem>();
	ecs.RegisterSystem<CelestialBodySystem>();
	ecs.RegisterSystem<PlaylistSystem>();
	ecs.RegisterSystem<DemoUISystem>();
}

//--------------------------
// MainFramework::OnInit
//
void MainFramework::OnInit()
{
	// scenes
	fw::UnifiedScene::Instance().GetEventDispatcher().Register( fw::E_SceneEvent::Activated, 
		fw::T_SceneEventCallback([this](fw::T_SceneEventFlags const flags, fw::SceneEventData const* const evnt)
			{
				ET_UNUSED(flags);
				ET_UNUSED(evnt);
				OnSceneActivated();
				DemoUI::OnSceneActivated();
			}));

	// gui
	DemoUI::SetupDataModels();

	// custom asset loading
	AssetPtr<TestData> m_TestAsset = core::ResourceManager::Instance()->GetAssetData<TestData>(core::HashString("test.txt"));
	ET_ASSERT(m_TestAsset != nullptr);

	ET_TRACE_I(ET_CTX_DEMO, "Custom Assets:\n\t%s", std::string(m_TestAsset->GetText(), m_TestAsset->GetLength()).c_str());
}

//--------------------------
// MainFramework::OnTick
//
// Demo specific updates independent of scene
//
void MainFramework::OnTick()
{
	fw::UnifiedScene& uniScene = fw::UnifiedScene::Instance();
	core::InputManager* const input = core::InputManager::GetInstance();

	// Scene switching
	//-----------------
	if(input->GetKeyState(E_KbdKey::F3) == E_KeyState::Pressed)
	{
		std::vector<core::HashString> const& scenes = GetSceneIds();
		if (--m_CurrentScene >= scenes.size())
		{
			m_CurrentScene = scenes.size() - 1;
		}

		uniScene.LoadScene(scenes[m_CurrentScene]);
	}

	if(input->GetKeyState(E_KbdKey::F4) == E_KeyState::Pressed)
	{
		std::vector<core::HashString> const& scenes = GetSceneIds();
		m_CurrentScene = (m_CurrentScene + 1) % scenes.size();

		uniScene.LoadScene(scenes[m_CurrentScene]);
	}

	// Screenshots
	//-------------
	if (input->GetKeyState(E_KbdKey::F12) == E_KeyState::Pressed)
	{
		m_ScreenshotCapture.Take(render::Viewport::GetCurrentViewport());
	}

	// view
	//------
	bool const up = (input->GetKeyState(E_KbdKey::Up) == E_KeyState::Down);
	if (up || (input->GetKeyState(E_KbdKey::Down) == E_KeyState::Down))
	{
		render::Scene& renderScene = uniScene.GetRenderScene();
		render::PostProcessingSettings ppSettings = renderScene.GetPostProcessingSettings();

		float const newExp = ppSettings.exposure * 4.f;
		ppSettings.exposure += (newExp - ppSettings.exposure) * TIME->DeltaTime() * (up ? 1.f : -1.f);

		ET_TRACE_I(ET_CTX_DEMO, "Exposure: %f", ppSettings.exposure);

		renderScene.SetPostProcessingSettings(ppSettings);
	}
}

//---------------------------------
// MainFramework::OnSceneActivated
//
// Keep track of current scene for switching and set up the camera
//
void MainFramework::OnSceneActivated()
{
	// active scene as loaded by boot config
	std::vector<core::HashString> const& scenes = GetSceneIds();
	auto foundSceneIt = std::find(scenes.cbegin(), scenes.cend(), fw::UnifiedScene::Instance().GetSceneId());
	ET_ASSERT(foundSceneIt != scenes.cend());
	m_CurrentScene = foundSceneIt - scenes.cbegin();

	// set up camera
	fw::EcsController& ecs = fw::UnifiedScene::Instance().GetEcs();
	fw::T_EntityId const camEnt = fw::UnifiedScene::Instance().GetActiveCamera();

	ET_ASSERT(camEnt != fw::INVALID_ENTITY_ID);
	ET_ASSERT(!ecs.HasComponent<FreeCameraComponent>(camEnt));

	ecs.AddComponents(camEnt, FreeCameraComponent());
}


} // namespace demo
} // namespace et
