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
	virtual ~I_RenderArea() = default;

	virtual void SetOnInit(std::function<void()>& callback) = 0;
	virtual void SetOnDeinit(std::function<void()>& callback) = 0;
	virtual void SetOnResize(std::function<void(vec2 const)>& callback) = 0;
	virtual void SetOnRender(std::function<void()>& callback) = 0;

	virtual void QueueDraw() = 0;
	virtual bool MakeCurrent() = 0;
};
