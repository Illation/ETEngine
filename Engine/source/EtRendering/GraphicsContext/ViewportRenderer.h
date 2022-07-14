#pragma once


namespace et {
namespace render {


//---------------------------------
// I_ViewportRenderer
//
// Interface for a class that can draw anything to a viewport - a scene, a UI element etc ....
//
class I_ViewportRenderer
{
public:
	I_ViewportRenderer() = default;
	virtual ~I_ViewportRenderer() = default;
	I_ViewportRenderer& operator=(const I_ViewportRenderer&) { return *this; }

	virtual rttr::type GetType() const = 0;
	virtual void OnInit() {}
	virtual void OnDeinit() {}
	virtual void OnResize(ivec2 const dim) { ET_UNUSED(dim); }
	virtual void OnRender(T_FbLoc const targetFb) {}
};


} // namespace render
} // namespace et
