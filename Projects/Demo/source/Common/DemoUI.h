#pragma once
#include <EtGUI/Context/DataModel.h>

#include <EtFramework/ECS/EcsController.h>
#include <EtFramework/ECS/ComponentView.h>
#include <EtFramework/Components/GuiCanvasComponent.h>


namespace et {
namespace demo {


struct CanvasView final : public fw::ComponentView
{
	CanvasView() : fw::ComponentView()
	{
		Declare(canvas);
	}

	WriteAccess<fw::GuiCanvasComponent> canvas;
};

//--------------
// DemoUISystem
//
// For toggling UI visibility
//
class DemoUISystem final : public fw::System<DemoUISystem, CanvasView>
{
public:
	DemoUISystem() = default; // no dependencies

	void Process(fw::ComponentRange<CanvasView>& range) override;
};

//--------------------------
// DemoUI
//
// Common class data for UI in the demo
//
class DemoUI final
{
public:
	static core::HashString const s_HelloWorldGuiId;

	//----------
	// GuiData
	//
	// Data model for the hello world screen
	//
	struct GuiData : public gui::I_DataModel
	{
		bool m_ShowText = true;
		std::string m_Animal = "dog";
	};

	static void SetupDataModels();
};


} // namespace demo
} // namespace et
