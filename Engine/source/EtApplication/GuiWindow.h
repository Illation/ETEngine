#pragma once
#include <EtCore/Util/GenericEventDispatcher.h>

#include <EtRHI/GraphicsContext/Viewport.h>

#include <EtApplication/Rendering/GlfwRenderWindow.h>
#include <EtApplication/Rendering/BasicGuiRenderer.h>
#include <EtApplication/Core/GlfwEventBase.h>


namespace et {
namespace app {


class GuiApplication;


//-----------
// GuiWindow
//
// Window including proper UI functionality based on lower level GUI systems
//
class GuiWindow : public GlfwEventBase
{
	// definitions
	//-------------

	friend class GuiApplication;
public:

	//---------------------------
	// E_RenderEvent
	//
	typedef uint8 T_EventFlags;
	enum E_Event : T_EventFlags
	{
		GW_Invalid = 0,

		GW_Maximize = 1 << 0,
		GW_Restore = 1 << 1,
		GW_GainFocus = 1 << 2,
		GW_LooseFocus = 1 << 3,

		GW_All = 0xFF
	};

	//---------------------------
	// EventData
	//
	struct EventData final
	{
	public:
		EventData(GuiWindow const* const window) : m_Window(ToPtr(window)) {}
		virtual ~EventData() = default;

		Ptr<GuiWindow const> const m_Window;
	};

private:
	typedef core::GenericEventDispatcher<T_EventFlags, EventData> T_EventDispatcher;

public:
	typedef T_EventDispatcher::T_CallbackId T_EventCallbackId;
	static constexpr T_EventCallbackId INVALID_CALLBACK = T_EventDispatcher::INVALID_ID;
	typedef T_EventDispatcher::T_CallbackFn T_EventCallback;

	// construct destruct
	//--------------------
	GuiWindow(core::WindowSettings const& settings);
	virtual ~GuiWindow();

private:
	void Init(); // called by the application
	void Deinit();

	// functionality
	//---------------
public:
	void SetGuiDocument(core::HashString const documentId);

	void Close();
	void ToggleMinimized();
	void ToggleMaximized();

	T_EventCallbackId RegisterCallback(T_EventFlags const flags, T_EventCallback& callback);
	void UnregisterCallback(T_EventCallbackId& callbackId);

	void StartDrag();
	void SetCustomBorderSize(int32 const size);

	// accessors
	//-----------
	GlfwRenderWindow& GetRenderWindow() { return m_RenderWindow; }
	gui::Context const& GetContext() { return m_GuiRenderer.GetContext(); }

	bool Focused() const;
	bool Maximized() const;
	bool Minimized() const;

	std::string const& GetTitle() const;


	// Data
	///////

private:
	GlfwRenderWindow m_RenderWindow;
	rhi::Viewport m_Viewport;
	BasicGuiRenderer m_GuiRenderer;

	T_EventDispatcher m_EventDispatcher;

	int32 m_CustomBorderSize = 0;

	bool m_IsInitialized = false;
};


} // namespace app
} // namespace et
