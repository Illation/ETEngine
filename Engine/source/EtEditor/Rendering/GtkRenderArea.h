#pragma once
#include <gtkmm/glarea.h>

#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>

#include <Engine/GraphicsHelper/RenderArea.h>


//---------------------------------
// GtkRenderArea
//
// Render area using GTKmm GLArea as a subsystem, managing an openGL context
//
class GtkRenderArea : public I_RenderArea
{
	// construct destruct
	//-------------------
public:
	GtkRenderArea(Gtk::GLArea* glArea);
	virtual ~GtkRenderArea() = default;

protected:
	void OnRealize();
	void OnUnrealize();
	void OnResize(int32 x, int32 y);
	bool OnRender(const Glib::RefPtr<Gdk::GLContext>& context);

	// Render Area Interface
	//-----------------------
	void SetOnInit(std::function<void(I_GraphicsApiContext* const)>& callback) override { m_OnInit = callback; }
	void SetOnDeinit(std::function<void()>& callback) override { m_OnDeinit = callback; }
	void SetOnResize(std::function<void(vec2 const)>& callback) override { m_OnResize = callback; }
	void SetOnRender(std::function<void()>& callback) override { m_OnRender = callback; }

	void QueueDraw() override;
	bool MakeCurrent() override;

	// Data
	///////
private:
	std::function<void(I_GraphicsApiContext* const)> m_OnInit;
	std::function<void()> m_OnDeinit;
	std::function<void(vec2 const)> m_OnResize;
	std::function<void()> m_OnRender;

	Gtk::GLArea* m_GlArea = nullptr;
};

