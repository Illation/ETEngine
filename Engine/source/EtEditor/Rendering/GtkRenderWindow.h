#pragma once

#include <gtkmm/window.h>
#include <glibmm/refptr.h>
#include <gdkmm/glcontext.h>

#include <EtRendering/GraphicsContext/RenderWindow.h>


namespace et {
namespace edit {


//---------------------------------
// GtkRenderWindow
//
// Creates context from a Gtk::Window
//
class GtkRenderWindow final : public render::RenderWindow
{
	// construct destruct
	//-------------------
public:
	GtkRenderWindow() : render::RenderWindow() {}
	~GtkRenderWindow();

	void SetSourceWindow(Gtk::Window* const window) { m_Source = window; }

	// accessors
	//-----------
	Glib::RefPtr<Gdk::GLContext> GetGdkContext() { return m_GdkContext; }

	bool UseDoubleBuffering() const { return m_UseDoubleBuffering; }
	bool UseDepthBuffer() const { return m_UseDepthBuffer; }
	bool UseStencilBuffer() const { return m_UseStencilBuffer; }

	// Render Window Interface
	//-------------------------
protected:
	render::I_GraphicsContextApi* CreateContext(render::GraphicsContextParams const& params) override;
	void SetCursorPos(ivec2 const pos) override;

	ivec2 GetDimensions() const override;
	bool HasFocus() const override;

	// Data
	///////
private:
	Gtk::Window* m_Source = nullptr;
	Glib::RefPtr<Gdk::GLContext> m_GdkContext;

	bool m_UseDoubleBuffering = false;
	bool m_UseDepthBuffer = false;
	bool m_UseStencilBuffer = false;

	render::I_GraphicsContextApi* m_Context = nullptr;
};


} // namespace edit
} // namespace et

