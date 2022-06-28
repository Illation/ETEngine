#pragma once



namespace et {
namespace gui {


//---------------------------------
// ContextRenderTarget
//
// Surface to draw UI to
//
class ContextRenderTarget final
{
public:
	// construct destruct
	//--------------------
	ContextRenderTarget() = default;
	ContextRenderTarget(ContextRenderTarget const& other);
	ContextRenderTarget& operator=(ContextRenderTarget const& other);
	~ContextRenderTarget();

	// functionality
	//---------------
	void UpdateForDimensions(ivec2 const dim);
	void DeleteFramebuffer();

	// accessors
	//-----------
	render::T_FbLoc GetFramebuffer() const { return m_Framebuffer; }
	render::TextureData const* GetTexture() const { return m_Texture.Get(); }
	ivec2 GetDimensions() const;

	// Data
	///////

private:
	render::T_FbLoc m_Framebuffer;
	render::T_RbLoc m_Renderbuffer;
	UniquePtr<render::TextureData> m_Texture;
};


} // namespace gui
} // namespace et
