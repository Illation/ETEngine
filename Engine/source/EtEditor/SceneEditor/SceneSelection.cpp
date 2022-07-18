#include <EtEditor/stdafx.h>
#include "SceneSelection.h"

#include <EtFramework/SceneGraph/UnifiedScene.h>
#include <EtFramework/Components/ModelComponent.h>

#include <EtRendering/Extensions/OutlineExtension.h>
#include <EtRendering/SceneStructure/RenderScene.h>


namespace et {
namespace edit {


//==========================
// Scene Selection
//==========================


//----------------------------------------------------
// SceneSelection::SetScene
//
// start listening for scene events
//
void SceneSelection::SetScene()
{
	fw::UnifiedScene::Instance().GetEventDispatcher().Register(fw::E_SceneEvent::All,
		fw::T_SceneEventCallback(std::bind(&SceneSelection::OnSceneEvent, this, std::placeholders::_1, std::placeholders::_2)));

	render::Scene& renderScene = fw::UnifiedScene::Instance().GetRenderScene();
	render::I_SceneExtension* const ext = renderScene.GetExtension(render::OutlineExtension::s_ExtensionId);
	if (ext == nullptr)
	{
		UniquePtr<render::OutlineExtension> outlineExt = Create<render::OutlineExtension>();
		m_OutlineExtension = ToPtr(outlineExt.Get());
		renderScene.AddExtension(UniquePtr<render::I_SceneExtension>::StaticCast(std::move(outlineExt)));
	}
	else
	{
		m_OutlineExtension = ToPtr(static_cast<render::OutlineExtension*>(ext));
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
		ET_LOG_W(ET_CTX_EDITOR, "SceneSelection::UnregisterListener > Listener not found");
		return;
	}

	core::RemoveSwap(m_Listeners, listenerIt); // the order of the listener list doesn't matter
}

//----------------------------------------------------
// SceneSelection::UnregisterListener
//
void SceneSelection::ClearSelection(bool const notify)
{
	m_SelectedEntities.clear();

	if (notify)
	{
		for (I_SceneSelectionListener* const listener : m_Listeners)
		{
			listener->OnEntitySelectionCleared();
		}
	}
}

//----------------------------------------------------
// SceneSelection::ToggleEntitySelected
//
// Selects the entity, or deselects if it already was selected. Notification can be disabled in case the triggerer is also a listener
//
void SceneSelection::ToggleEntitySelected(fw::T_EntityId const entity, bool const notify)
{
	auto entityIt = std::find(m_SelectedEntities.begin(), m_SelectedEntities.end(), entity);

	bool const selected = (entityIt == m_SelectedEntities.end());

	if (selected)
	{
		m_SelectedEntities.emplace_back(entity);
	}
	else
	{
		m_SelectedEntities.erase(entityIt);
	}

	if (notify)
	{
		for (I_SceneSelectionListener* const listener : m_Listeners)
		{
			listener->OnEntitySelectionChanged(entity, selected);
		}
	}
}

//----------------------------------------------------
// SceneSelection::Pick
//
// try picking an entity from within the viewport
//
void SceneSelection::Pick(ivec2 const pos, render::Viewport* const viewport, bool const add)
{
	if (!add)
	{
		ClearSelection(true);
	}

	m_IdRenderer.Pick(pos, viewport, std::function<void(fw::T_EntityId const)>([this](fw::T_EntityId const pickResult)
		{
			if (pickResult != fw::INVALID_ENTITY_ID)
			{
				ToggleEntitySelected(pickResult, true);
			}
		}));
}

//----------------------------------------------------
// SceneSelection::UpdateOutliners
//
void SceneSelection::UpdateOutlines() const
{
	// actually we should do this for every viewport with a scene renderer
	if (m_OutlineExtension != nullptr)
	{
		m_OutlineExtension->Clear();
		m_OutlineExtension->SetColor(m_OutlineColor);
		for (fw::T_EntityId const entity : m_SelectedEntities)
		{
			RecursiveAddOutlines(entity);
		}
	}
}

//----------------------------------------------------
// SceneSelection::OnSceneEvent
//
// pass the event through to any listeners
//
void SceneSelection::OnSceneEvent(fw::T_SceneEventFlags const flags, fw::SceneEventData const* const eventData)
{
	for (I_SceneSelectionListener* const listener : m_Listeners)
	{
		listener->OnSceneEvent(static_cast<fw::E_SceneEvent>(flags), eventData);
	}

	if ((flags == fw::E_SceneEvent::Activated) && !m_IsIdRendererInitialized)
	{
		m_IdRenderer.Initialize();
		m_IsIdRendererInitialized = true;
	}
}

//----------------------------------------------------
// SceneSelection::OnSceneEvent
//
void SceneSelection::RecursiveAddOutlines(fw::T_EntityId const entity) const
{
	fw::EcsController const& ecs = fw::UnifiedScene::Instance().GetEcs();

	if (ecs.HasComponent<fw::ModelComponent>(entity) && ecs.HasComponent<fw::TransformComponent>(entity))
	{
		m_OutlineExtension->AddMesh(ecs.GetComponent<fw::ModelComponent>(entity).GetMesh(), 
			ecs.GetComponent<fw::TransformComponent>(entity).GetNodeId());
	}

	std::vector<fw::T_EntityId> const& children = ecs.GetChildren(entity);
	for (fw::T_EntityId const child : children)
	{
		RecursiveAddOutlines(child);
	}
}


} // namespace edit
} // namespace et
