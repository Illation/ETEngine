#pragma once

#include <gtkmm/window.h>
#include <glibmm/refptr.h>
#include <gdkmm/glcontext.h>

#include <EtRHI/GraphicsContext/RenderWindow.h>


namespace et {
namespace edit {


//---------------------------------
// GtkRenderWindow
//
// Creates context from a Gtk::Window
//
class GtkRenderWindow final : public rhi::RenderWindow
{
	// construct destruct
	//-------------------
public:
	GtkRenderWindow() : rhi::RenderWindow() {}
	~GtkRenderWindow() = default;

	void SetSourceWindow(Ptr<Gtk::Window> const window) { m_Source = window; }

	// accessors
	//-----------
	Glib::RefPtr<Gdk::GLContext> GetGdkContext() { return m_GdkContext; }

	bool UseDoubleBuffering() const { return m_UseDoubleBuffering; }
	bool UseDepthBuffer() const { return m_UseDepthBuffer; }
	bool UseStencilBuffer() const { return m_UseStencilBuffer; }

	// Render Window Interface
	//-------------------------
protected:
	Ptr<rhi::I_RenderDevice> CreateRenderDevice(rhi::RenderDeviceParams const& params) override;
	void SetCursorPos(ivec2 const pos) override;

	ivec2 GetDimensions() const override;
	bool HasFocus() const override;

	// Data
	///////
private:
	Ptr<Gtk::Window> m_Source;
	Glib::RefPtr<Gdk::GLContext> m_GdkContext;

	bool m_UseDoubleBuffering = false;
	bool m_UseDepthBuffer = false;
	bool m_UseStencilBuffer = false;

	UniquePtr<rhi::I_RenderDevice> m_RenderDevice;
};


} // namespace edit
} // namespace et

