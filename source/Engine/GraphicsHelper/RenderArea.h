#pragma once

#include <functional>


//---------------------------------
// I_RenderArea
//
// Interface for a class that manages an rendering context and surface to render to
//
class I_RenderArea
{
public:
	I_RenderArea() = default;
	virtual ~I_RenderArea() = default;
	I_RenderArea& operator=(const I_RenderArea&) { return *this; }

	virtual void SetOnInit(std::function<void()>& lambda) = 0;
	virtual void SetOnDeinit(std::function<void()>& lambda) = 0;
	virtual void SetOnResize(std::function<void(vec2 const)>& lambda) = 0;
	virtual void SetOnRender(std::function<void()>& lambda) = 0;

	virtual void QueueDraw() = 0;
	virtual bool MakeCurrent() = 0;
};
