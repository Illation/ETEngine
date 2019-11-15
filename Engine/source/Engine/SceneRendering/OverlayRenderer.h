#pragma once


namespace render {


//---------------------------------
// I_OverlayRenderer
//
// Interface for a class that can draw overlays
//
class I_OverlayRenderer
{
public:
	virtual ~I_OverlayRenderer() = default;

	virtual void DrawOverlays(T_FbLoc const targetFb) = 0;
};


}

