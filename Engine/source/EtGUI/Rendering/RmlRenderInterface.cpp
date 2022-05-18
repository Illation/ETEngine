#include "stdafx.h"
#include "RmlRenderInterface.h"


namespace et {
namespace gui {


//======================
// RML Render Interface
//======================


//------------------------------------
// RmlRenderInterface::RenderGeometry
//
// Simple implementation for rendering geometry
//
void RmlRenderInterface::RenderGeometry(Rml::Vertex* vertices, 
	int32 numVertices, 
	int32* indices, 
	int32 numIndices, 
	Rml::TextureHandle texture, 
	Rml::Vector2f const& translation)
{
	UNUSED(vertices);
	UNUSED(numVertices);
	UNUSED(indices);
	UNUSED(numIndices);
	UNUSED(texture);
	UNUSED(translation);
}

//-----------------------------------------
// RmlRenderInterface::EnableScissorRegion
//
// Called by RmlUi when it wants to enable or disable scissoring to clip content
//
void RmlRenderInterface::EnableScissorRegion(bool enable)
{
	UNUSED(enable);
}

//--------------------------------------
// RmlRenderInterface::SetScissorRegion
//
// Called by RmlUi when it wants to change the scissor region
//
void RmlRenderInterface::SetScissorRegion(int32 x, int32 y, int32 width, int32 height)
{
	UNUSED(x);
	UNUSED(y);
	UNUSED(width);
	UNUSED(height);
}


} // namespace gui
} // namespace et

