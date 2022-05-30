#pragma once
#include "FreeCamera.h"

#include <EtCore/Content/AssetPointer.h>

#include <EtFramework/SceneGraph/SceneEvents.h>

#include <EtRuntime/AbstractFramework.h>


namespace et { namespace gui {
	class SdfFont;
} }


namespace et {
namespace demo {


//--------------------------
// MainFramework
//
// User facing wrapper around the engine
//
class MainFramework final : public rt::AbstractFramework
{
	static core::HashString const s_HelloWorldGuiId;

	struct GuiData
	{
		bool m_ShowText = true;
		std::string m_Animal = "dog";
	};

	// construct destruct
	//--------------------
public:
	MainFramework() : AbstractFramework() {} // initializes the engine
	~MainFramework() = default;

	// framework interface
	//---------------------
private:
	void OnSystemInit() override;
	void OnInit() override;
	void OnTick() override;

	// utility
	//---------
	void OnSceneActivated();
	void PreLoadGUI(fw::SceneEventPreLoadGUIData const* const evnt);
	void PostLoadGUI(fw::SceneEventGUIData const* const evnt);


	// Data
	///////

	size_t m_CurrentScene = 0u;

	AssetPtr<gui::SdfFont> m_DebugFont;
	bool m_DrawDebugInfo = true;
	bool m_DrawFontAtlas = false;

	GuiData m_GuiData;
	bool m_ShowGui = true;
};


} // namespace demo
} // namespace et
