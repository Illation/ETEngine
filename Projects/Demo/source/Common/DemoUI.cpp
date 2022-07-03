#include "stdafx.h"
#include "DemoUI.h"

#include <RmlUi/Core/ElementDocument.h>

#include <EtGUI/Context/RmlGlobal.h>
#include <EtGUI/GuiExtension.h>

#include <EtFramework/SceneGraph/UnifiedScene.h>


namespace et {
namespace demo {


//=========
// Demo UI
//=========


//--------------------------
// DemoUISystem::Process
//
void DemoUISystem::Process(fw::ComponentRange<CanvasView>& range)
{
	if (core::InputManager::GetInstance()->GetKeyState(E_KbdKey::U) == E_KeyState::Pressed)
	{
		for (CanvasView& view : range)
		{
			view.canvas->SetActive(!view.canvas->IsActive());
		}
	}
}

// static
core::HashString const DemoUI::s_HelloWorldGuiId("GUI/hello_world.rml");


//--------------------------
// DemoUI::SetupDataModels
//
// Ensure data models for GUI canvases we use are present when the components load
//
void DemoUI::SetupDataModels()
{
	gui::RmlGlobal::GetDataModelFactory().RegisterInstancer("animals", gui::DataModelFactory::T_InstanceFn(
		[](Rml::DataModelConstructor modelConstructor) -> RefPtr<gui::I_DataModel>
		{
			RefPtr<GuiData> ret = Create<GuiData>();
			modelConstructor.Bind("show_text", &ret->m_ShowText);
			modelConstructor.Bind("animal", &ret->m_Animal);
			return std::move(ret);
		}));
}

//--------------------------
// DemoUI::OnSceneActivated
//
// Modify some UI elements to look nicer
//
void DemoUI::OnSceneActivated()
{
	fw::UnifiedScene::Instance().GetEcs().ProcessViewOneShot(fw::T_OneShotProcess<CanvasView>([](fw::ComponentRange<CanvasView>& range)
		{
			gui::ContextContainer& guiContainer = fw::UnifiedScene::Instance().GetGuiExtension()->GetContextContainer();
			for (CanvasView& view : range)
			{
				if (view.canvas->GetGuiDocumentId() == DemoUI::s_HelloWorldGuiId)
				{
					Rml::ElementDocument* const doc = guiContainer.GetDocument(view.canvas->GetId());
					ET_ASSERT(doc != nullptr);

					Rml::Element* const element = doc->GetElementById("world");
					if (element != nullptr)
					{
						element->SetInnerRML(reinterpret_cast<const char*>(u8"🌍"));
						element->SetProperty("font-size", "1.5em");
					}
				}
			}
		}));
}


} // namespace demo
} // namespace et
