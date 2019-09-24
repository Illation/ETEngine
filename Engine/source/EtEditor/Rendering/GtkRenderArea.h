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

	// Render Area Interface
	//-----------------------
protected:
	void SetOnInit(std::function<void(I_GraphicsApiContext* const)>& callback) override;	
	void SetOnDeinit(std::function<void()>& callback) override;
	void SetOnResize(std::function<void(vec2 const)>& callback) override;
	void SetOnRender(std::function<void()>& callback) override;

	void QueueDraw() override;
	bool MakeCurrent() override;

	// Data
	///////
private:

	Gtk::GLArea* m_GlArea = nullptr;
};

