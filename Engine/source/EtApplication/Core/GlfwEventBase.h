#pragma once
#include <EtCore/Input/CursorShapes.h>
#include <EtCore/Input/ClipboardControllerInterface.h>
#include <EtCore/Input/RawInputProvider.h>

#include <EtApplication/Rendering/GlfwRenderArea.h>


// fwd
class GLFWcursor;


namespace et {
namespace app {


//----------------------------
// GlfwEventBase
//
// Handles system and input events for a GlfwRenderArea/Window
//
class GlfwEventBase :
	public core::I_CursorShapeManager,
	public core::I_ClipboardController
{
	// static
	//--------
	static E_MouseButton GetButtonFromGlfw(int32 const bottonCode);

public:
	// construct destruct
	//--------------------
	GlfwEventBase() = default;
	virtual ~GlfwEventBase();

	void InitEvents(Ptr<GlfwRenderArea> const renderArea);

	// accessors
	//-----------
	core::RawInputProvider& GetInputProvider() { return m_InputProvider; }
	core::T_KeyModifierFlags GetCurrentModifiers() const { return m_CurrentModifiers; }
	GlfwRenderArea* GetRenderArea() { return m_RenderArea.Get(); }

	// interfaces
	//------------
private:
	// cursor shape manager
	bool SetCursorShape(core::E_CursorShape const shape) override;

	// clipboard controller
	void SetClipboardText(std::string const& textUtf8) override;
	void GetClipboardText(std::string& outTextUtf8) override;

	// utility
	//---------
	void SetCurrentModifiers(core::T_KeyModifierFlags const mods) { m_CurrentModifiers = mods; }


	// Data
	///////

	Ptr<GlfwRenderArea> m_RenderArea;

	core::RawInputProvider m_InputProvider;
	core::T_KeyModifierFlags m_CurrentModifiers;

	std::unordered_map<core::E_CursorShape, Ptr<GLFWcursor>> m_CursorMap;
};


} // namespace app
} // namespace et

