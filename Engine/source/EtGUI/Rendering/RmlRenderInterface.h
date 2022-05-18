#pragma once
#include <RmlUi/Core/RenderInterface.h>


namespace et {
namespace gui {


//---------------------------------
// RmlSystemInterface
//
// Implementation of RmlUi's system interface
//
class RmlRenderInterface final : public Rml::RenderInterface
{
public:
	// construct destruct
	//--------------------
	RmlRenderInterface() : Rml::RenderInterface() {}
	~RmlRenderInterface() = default;

	// interface implementation
	//--------------------------
	void RenderGeometry(Rml::Vertex* vertices, 
		int32 numVertices, 
		int32* indices,
		int32 numIndices,
		Rml::TextureHandle texture, 
		Rml::Vector2f const& translation) override;

	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(int32 x, int32 y, int32 width, int32 height) override;
};


} // namespace gui
} // namespace et

