#pragma once
#include <EtCore/Input/RawInputListener.h>

#include <EtRHI/GraphicsContext/ViewportRenderer.h>

#include <EtGUI/Context/Context.h>
#include <EtGUI/Rendering/GuiRenderer.h>


// fwd
namespace et { namespace core {
	class RawInputProvider;
} }


namespace et {
namespace app {


//------------------
// BasicGuiRenderer
//
// Viewport renderer for 2D GUI contexts
//
class BasicGuiRenderer final : public rhi::I_ViewportRenderer, public core::I_RawInputListener
{
	// construct destruct
	//--------------------
public:
	BasicGuiRenderer() : rhi::I_ViewportRenderer() {}
	~BasicGuiRenderer();

	// functionality
	//---------------
	void Init(Ptr<core::RawInputProvider> const inputProvider);
	void Deinit();

	void SetGuiDocument(core::HashString const documentId);

	gui::Context& GetContext() { return m_GuiContext; }
	gui::Context const& GetContext() const { return m_GuiContext; }

	// Viewport Renderer Interface
	//-----------------------------
private:
	rttr::type GetType() const override { return rttr::type::get<BasicGuiRenderer>(); }
	void OnInit() override {}
	void OnDeinit() override {}
	void OnResize(ivec2 const dim) override;
	void OnRender(rhi::T_FbLoc const targetFb) override;

	// input listener interface
	//--------------------------
	int8 GetPriority() const override { return 1; }
	bool ProcessKeyPressed(E_KbdKey const key, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessKeyReleased(E_KbdKey const key, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessMousePressed(E_MouseButton const button, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessMouseReleased(E_MouseButton const button, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessMouseMove(ivec2 const& mousePos, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessMouseWheelDelta(ivec2 const& mouseWheel, core::T_KeyModifierFlags const modifiers) override;
	bool ProcessTextInput(core::E_Character const character) override;


	// Data
	///////

	Ptr<core::RawInputProvider> m_InputProvider; // if not null we are initialized

	gui::Context m_GuiContext;
	gui::ContextRenderTarget m_ContextRenderTarget;
	gui::GuiRenderer m_GuiRenderer;
};


} // namespace app
} // namespace et

