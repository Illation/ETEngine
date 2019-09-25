#pragma once


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

	virtual void OnInit() {}
	virtual void OnDeinit() {}
	virtual void OnResize(ivec2 const dim) { UNUSED(dim); }
	virtual void OnRender(T_FbLoc const targetFb) {}
};

