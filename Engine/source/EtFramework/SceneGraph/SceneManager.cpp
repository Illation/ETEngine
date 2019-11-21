#include "stdafx.h"
#include "SceneManager.h"

#include <algorithm>

#include "AbstractScene.h"


SceneManager::~SceneManager()
{
	for (AbstractScene* scene : m_Scenes)
	{
		delete scene;
		scene = nullptr;
	}
}

void SceneManager::AddScene(AbstractScene* const scene)
{
	auto it = find(m_Scenes.begin(), m_Scenes.end(), scene);

	if (it == m_Scenes.end())
	{
		m_Scenes.push_back(scene);

		if ((m_ActiveScene == nullptr) && (m_NewActiveScene == nullptr))
		{
			SetNewScene(scene);
		}
	}
}

void SceneManager::RemoveScene(AbstractScene* const scene)
{
	auto it = find(m_Scenes.begin(), m_Scenes.end(), scene);

	if (it != m_Scenes.end())
	{
		m_Scenes.erase(it);
	}
}

void SceneManager::SetActiveGameScene(std::string const& sceneName)
{
	auto it = find_if(m_Scenes.begin(), m_Scenes.end(), [&sceneName](AbstractScene* scene)
		{
			return scene->GetName() == sceneName;
		});

	if (it != m_Scenes.end())
	{
		SetNewScene(*it);
	}
}

void SceneManager::NextScene()
{
	for (size_t i = 0; i < m_Scenes.size(); ++i)
	{
		if (m_Scenes[i] == m_ActiveScene)
		{
			size_t nextSceneIdx = (++i) % m_Scenes.size();

			SetNewScene(m_Scenes[nextSceneIdx]);
			return;
		}
	}
}

void SceneManager::PreviousScene()
{
	for (size_t i = 0; i < m_Scenes.size(); ++i)
	{
		if (m_Scenes[i] == m_ActiveScene)
		{
			if (i == 0)
			{
				i = m_Scenes.size();
			}

			--i;

			SetNewScene(m_Scenes[i]);
			return;
		}
	}
}

void SceneManager::OnTick()
{
	if (m_NewActiveScene != nullptr)
	{
		//Deactivate the current active scene
		if (m_ActiveScene != nullptr)
		{
			m_ActiveScene->RootDeactivate();
			m_EventDispatcher.Notify(E_SceneEvent::Deactivated, new SceneEventData(m_ActiveScene));
		}

		//Set New Scene
		m_ActiveScene = m_NewActiveScene;
		m_NewActiveScene = nullptr;

		//Active the new scene and reset SceneTimer
		LOG(std::string("Switching to scene: ") + m_ActiveScene->m_Name);
		m_ActiveScene->RootActivate();
		m_EventDispatcher.Notify(E_SceneEvent::Activated, new SceneEventData(m_ActiveScene));
	}

	if (m_ActiveScene != nullptr)
	{
		m_ActiveScene->RootUpdate();
	}
}

void SceneManager::SetNewScene(AbstractScene* const scene)
{
	m_NewActiveScene = scene;
	m_EventDispatcher.Notify(E_SceneEvent::SceneSwitch, new SceneEventData(m_NewActiveScene));
}
