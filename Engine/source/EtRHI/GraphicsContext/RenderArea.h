#pragma once

#include <functional>


namespace et {
namespace rhi {


//---------------------------------
// I_RenderArea
//
// Interface for a class that manages an rendering context and surface to rhi to
//
class I_RenderArea
{
public:
	virtual ~I_RenderArea() = default;

	virtual void SetOnInit(std::function<void(Ptr<I_RenderDevice> const)>& callback) = 0;
	virtual void SetOnDeinit(std::function<void()>& callback) = 0;
	virtual void SetOnResize(std::function<void(vec2 const)>& callback) = 0;
	virtual void SetOnRender(std::function<void(T_FbLoc const)>& callback) = 0;

	virtual void QueueDraw() = 0;
	virtual bool MakeCurrent() = 0;

	virtual ivec2 GetDimensions() const = 0;
};


} // namespace rhi
} // namespace et
