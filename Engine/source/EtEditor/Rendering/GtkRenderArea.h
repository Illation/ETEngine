#pragma once
#include <gtkmm/glarea.h>
#include <gtkmm/builder.h>

#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>

#include <EtRHI/GraphicsContext/RenderArea.h>


namespace et {
namespace edit {


//---------------------------------
// SingleContextGlArea
//
// Inherit from default GLArea to ensure all surfaces use the same openGL context
//
class SingleContextGlArea final : public Gtk::GLArea
{
	// construct destruct
	//-------------------
public:
	SingleContextGlArea();
	SingleContextGlArea(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder);

	// signals
	//-----------------------
private:
	Glib::RefPtr<Gdk::GLContext> OnCreateContext();
};


//---------------------------------
// GtkRenderArea
//
// Render area using GTKmm GLArea as a subsystem, managing an openGL context
//
class GtkRenderArea : public rhi::I_RenderArea
{
	// construct destruct
	//-------------------
public:
	GtkRenderArea(Ptr<SingleContextGlArea> const glArea);
	virtual ~GtkRenderArea() = default;

	// signals
	//-----------------------
protected:
	void OnRealize();
	void OnUnrealize();
	void OnResize(int32 x, int32 y);
	bool OnRender(Glib::RefPtr<Gdk::GLContext> const& context);

	// Render Area Interface
	//-----------------------
	void SetOnInit(std::function<void(Ptr<rhi::I_RenderDevice> const)>& callback) override { m_OnInit = callback; }
	void SetOnDeinit(std::function<void()>& callback) override { m_OnDeinit = callback; }
	void SetOnResize(std::function<void(vec2 const)>& callback) override { m_OnResize = callback; }
	void SetOnRender(std::function<void(rhi::T_FbLoc const)>& callback) override { m_OnRender = callback; }

	void QueueDraw() override;
	bool MakeCurrent() override;

	ivec2 GetDimensions() const override;

	// Data
	///////
private:
	std::function<void(Ptr<rhi::I_RenderDevice> const)> m_OnInit;
	std::function<void()> m_OnDeinit;
	std::function<void(vec2 const)> m_OnResize;
	std::function<void(rhi::T_FbLoc const)> m_OnRender;

	Ptr<SingleContextGlArea> m_GlArea;
};


} // namespace edit
} // namespace et
