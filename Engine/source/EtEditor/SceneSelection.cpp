#include "stdafx.h"
#include "SceneSelection.h"

#include <Engine/SceneGraph/AbstractScene.h>
#include <Engine/SceneGraph/Entity.h>
#include <Engine/GraphicsHelper/SceneRenderer.h>
#include <Engine/GraphicsHelper/OutlineRenderer.h>


//==========================
// Scene Selection
//==========================


//----------------------------------------------------
// SceneSelection::SetScene
//
// start listening for scene events
//
void SceneSelection::SetScene(AbstractScene* const scene)
{
	m_Scene = scene;

	m_Scene->GetEventDispatcher().Register(E_SceneEvent::All, T_SceneEventCallback(std::bind(&SceneSelection::OnSceneEvent, this, std::placeholders::_1)));

	if (!m_IsIdRendererInitialized)
	{
		m_IdRenderer.Initialize();
		m_IsIdRendererInitialized = true;
	}
}

//----------------------------------------------------
// SceneSelection::RegisterListener
//
void SceneSelection::RegisterListener(I_SceneSelectionListener* const listener)
{
	ET_ASSERT(std::find(m_Listeners.cbegin(), m_Listeners.cend(), listener) == m_Listeners.cend(), "Listener already registered!");

	m_Listeners.emplace_back(listener);
}

//----------------------------------------------------
// SceneSelection::UnregisterListener
//
void SceneSelection::UnregisterListener(I_SceneSelectionListener const* const listener)
{
	// try finding the listener
	auto listenerIt = std::find(m_Listeners.begin(), m_Listeners.end(), listener);

	// it should have been found
	if (listenerIt == m_Listeners.cend())
	{
		LOG("SceneSelection::UnregisterListener > Listener not found", LogLevel::Warning);
		return;
	}

	// swap and remove - the order of the listener list doesn't matter
	if (m_Listeners.size() > 1u)
	{
		std::iter_swap(listenerIt, std::prev(m_Listeners.end()));
		m_Listeners.pop_back();
	}
	else
	{
		m_Listeners.clear();
	}
}

//----------------------------------------------------
// SceneSelection::UnregisterListener
//
void SceneSelection::ClearSelection()
{
	m_SelectedEntities.clear();
}

//----------------------------------------------------
// SceneSelection::AddItemToSelection
//
void SceneSelection::AddItemToSelection(Entity* const entity)
{
	m_SelectedEntities.emplace_back(entity);
}

//----------------------------------------------------
// SceneSelection::Pick
//
// try picking an entity from within the viewport
//
void SceneSelection::Pick(ivec2 const pos, Viewport* const viewport, bool const add)
{
	if (!add)
	{
		m_SelectedEntities.clear();
	}

	m_IdRenderer.Pick(pos, viewport, std::function<void(Entity* const)>([this](Entity* const pickResult)
		{
			if (pickResult != nullptr)
			{
				AddItemToSelection(pickResult);
			}
		}));
}

//----------------------------------------------------
// SceneSelection::OnSceneEvent
//
// pass the event through to any listeners
//
void SceneSelection::OnSceneEvent(SceneEventData const* const eventData)
{
	for (I_SceneSelectionListener* const listener : m_Listeners)
	{
		listener->OnSceneEvent(eventData);
	}
}

//----------------------------------------------------
// SceneSelection::OnTick
//
void SceneSelection::OnTick()
{
	if (m_SelectedEntities.empty())
	{
		return;
	}

	OutlineRenderer& outlineRenderer = SceneRenderer::GetInstance()->GetOutlineRenderer();

	outlineRenderer.SetColor(m_OutlineColor);
	outlineRenderer.AddEntities(m_SelectedEntities);
}
