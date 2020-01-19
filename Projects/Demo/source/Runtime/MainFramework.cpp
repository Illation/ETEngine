#include "stdafx.h"
#include "MainFramework.h"

#include "SpawnSystem.h"
#include "LightControlSystem.h"
#include "SwirlyLightSystem.h"
#include "CelestialBodySystem.h"
#include "PlaylistSystem.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/SpriteFont.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>
#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Audio/AudioManager.h>


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
}

//--------------------------
// MainFramework::OnInit
//
void MainFramework::OnInit()
{
	// Fonts
	m_DebugFont = ResourceManager::Instance()->GetAssetData<SpriteFont>("Ubuntu-Regular.ttf"_hash);

	// scenes
	fw::UnifiedScene& uniScene = fw::UnifiedScene::Instance();

	m_Scenes.push_back(GetHash("EditorScene.json"));
	m_Scenes.push_back(GetHash("PlanetScene.json"));
	m_Scenes.push_back(GetHash("PhysicsScene.json"));
	
	uniScene.GetEventDispatcher().Register(E_SceneEvent::Activated,
		T_SceneEventCallback([this](T_SceneEventFlags const flags, SceneEventData const* const evnt)
		{
			UNUSED(flags);
			UNUSED(evnt);

			// active scene as loaded by user prefs
			auto foundSceneIt = std::find(m_Scenes.cbegin(), m_Scenes.cend(), fw::UnifiedScene::Instance().GetSceneId());
			ET_ASSERT(foundSceneIt != m_Scenes.cend());
			m_CurrentScene = foundSceneIt - m_Scenes.cbegin();

			// set up camera
			fw::EcsController& ecs = fw::UnifiedScene::Instance().GetEcs();
			fw::T_EntityId const camEnt = fw::UnifiedScene::Instance().GetActiveCamera();

			ET_ASSERT(camEnt != fw::INVALID_ENTITY_ID);
			ET_ASSERT(!ecs.HasComponent<FreeCameraComponent>(camEnt));

			ecs.AddComponents(camEnt, FreeCameraComponent());
		}));

	// audio
	AudioManager::GetInstance()->SetDistanceModel(AL_INVERSE_DISTANCE);
}

//--------------------------
// MainFramework::OnTick
//
// Demo specific updates indepenant of scene
//
void MainFramework::OnTick()
{
	fw::UnifiedScene& uniScene = fw::UnifiedScene::Instance();
	InputManager* const input = InputManager::GetInstance();

	// Scene switching
	//-----------------
	if(input->GetKeyState(E_KbdKey::F3) == E_KeyState::Pressed)
	{
		if (--m_CurrentScene >= m_Scenes.size())
		{
			m_CurrentScene = m_Scenes.size() - 1;
		}

		uniScene.LoadScene(m_Scenes[m_CurrentScene]);
	}

	if(input->GetKeyState(E_KbdKey::F4) == E_KeyState::Pressed)
	{
		m_CurrentScene = (m_CurrentScene + 1) % m_Scenes.size();

		uniScene.LoadScene(m_Scenes[m_CurrentScene]);
	}

	// Screenshots
	//-------------
	if (input->GetKeyState(E_KbdKey::F12) == E_KeyState::Pressed)
	{
		m_ScreenshotCapture.Take(Viewport::GetCurrentViewport());
	}

	// view
	//------
	bool const up = (input->GetKeyState(E_KbdKey::Up) == E_KeyState::Down);
	if (up || (input->GetKeyState(E_KbdKey::Down) == E_KeyState::Down))
	{
		render::Scene& renderScene = uniScene.GetRenderScene();
		PostProcessingSettings ppSettings = renderScene.GetPostProcessingSettings();

		float const newExp = ppSettings.exposure * 4.f;
		ppSettings.exposure += (newExp - ppSettings.exposure) * TIME->DeltaTime() * (up ? 1.f : -1.f);

		LOG(FS("Exposure: %f", ppSettings.exposure));

		renderScene.SetPostProcessingSettings(ppSettings);
	}

	// debug info
	//------------
	if (input->GetKeyState(E_KbdKey::H) == E_KeyState::Pressed)
	{
		m_DrawDebugInfo = !m_DrawDebugInfo;
	}

	if (input->GetKeyState(E_KbdKey::J) == E_KeyState::Pressed)
	{
		m_DrawFontAtlas = !m_DrawFontAtlas;
	}

	I_ViewportRenderer* const viewRenderer = Viewport::GetCurrentViewport()->GetViewportRenderer();
	if (viewRenderer != nullptr && viewRenderer->GetType() == typeid(render::ShadedSceneRenderer))
	{
		render::ShadedSceneRenderer* const sceneRenderer = static_cast<render::ShadedSceneRenderer*>(viewRenderer);

		if (m_DrawDebugInfo)
		{
			TextRenderer& textRenderer = sceneRenderer->GetTextRenderer();

			textRenderer.SetFont(m_DebugFont.get());
			textRenderer.SetColor(vec4(1, 0.3f, 0.3f, 1));
			std::string outString = "FPS: " + std::to_string(PERFORMANCE->GetRegularFPS());
			textRenderer.DrawText(outString, vec2(20, 20 + (m_DebugFont->GetFontSize()*1.1f) * 1));
			textRenderer.SetColor(vec4(1, 1, 1, 1));
			outString = "Frame ms: " + std::to_string(PERFORMANCE->GetFrameMS());
			textRenderer.DrawText(outString, vec2(20, 20 + (m_DebugFont->GetFontSize()*1.1f) * 2));
			outString = "Draw Calls: " + std::to_string(PERFORMANCE->m_PrevDrawCalls);
			textRenderer.DrawText(outString, vec2(20, 100 + (m_DebugFont->GetFontSize()*1.1f) * 3));
		}

		if (m_DrawFontAtlas)
		{
			SpriteRenderer::E_ScalingMode const scalingMode = SpriteRenderer::E_ScalingMode::TextureAbs;
			sceneRenderer->GetSpriteRenderer().Draw(m_DebugFont->GetAtlas(), vec2(1000, 0), vec4(1), vec2(0), vec2(1), 0, 0, scalingMode);
		}
	}
}


} // namespace demo
} // namespace et
