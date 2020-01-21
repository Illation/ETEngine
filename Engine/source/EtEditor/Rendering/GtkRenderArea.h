#pragma once
#include <gtkmm/glarea.h>
#include <gtkmm/builder.h>

#include <EtCore/UpdateCycle/RealTimeTickTriggerer.h>

#include <EtRendering/GraphicsContext/RenderArea.h>


namespace et {
namespace edit {


//---------------------------------
// SingleContextGlArea
//
// Inherit from default GLArea to ensure all surfaces use the same openGL context
//
class SingleContextGlArea final : public Gtk::GLArea
{
	// definitions
	//-------------------
private:
	//---------------------------------
	// SingleContextCache
	//
	// Ensures all rendering areas in the editor use the same open GL context
	//
	struct ContextCache
	{
		Glib::RefPtr<Gdk::GLContext> glibContext;
		I_GraphicsApiContext* apiContext;
	};

	static ContextCache* s_SingleContextCache;

public:
	static void DestroyContext();

	// construct destruct
	//-------------------
	SingleContextGlArea();
	SingleContextGlArea(BaseObjectType* cobject, Glib::RefPtr<Gtk::Builder> const& refBuilder);

	// accessors
	//-----------
	I_GraphicsApiContext* GetApiContext() const;

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
class GtkRenderArea : public I_RenderArea
{
	// construct destruct
	//-------------------
public:
	GtkRenderArea(SingleContextGlArea* const glArea);
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
	void SetOnInit(std::function<void(I_GraphicsApiContext* const)>& callback) override { m_OnInit = callback; }
	void SetOnDeinit(std::function<void()>& callback) override { m_OnDeinit = callback; }
	void SetOnResize(std::function<void(vec2 const)>& callback) override { m_OnResize = callback; }
	void SetOnRender(std::function<void(T_FbLoc const)>& callback) override { m_OnRender = callback; }

	void QueueDraw() override;
	bool MakeCurrent() override;

	ivec2 GetDimensions() const override;

	// Data
	///////
private:
	std::function<void(I_GraphicsApiContext* const)> m_OnInit;
	std::function<void()> m_OnDeinit;
	std::function<void(vec2 const)> m_OnResize;
	std::function<void(T_FbLoc const)> m_OnRender;

	SingleContextGlArea* m_GlArea = nullptr;
};


} // namespace edit
} // namespace et
