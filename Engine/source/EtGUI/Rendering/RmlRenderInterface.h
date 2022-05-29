#pragma once
#include <RmlUi/Core/RenderInterface.h>

#include <EtCore/Content/AssetPointer.h>

#include <EtRendering/GraphicsContext/GraphicsTypes.h>
#include <EtRendering/GraphicsTypes/TextureData.h>
#include <EtRendering/GraphicsTypes/Shader.h>


namespace et {
namespace gui {


//---------------------------------
// RmlRenderInterface
//
// Implementation of RmlUi's render interface
//
class RmlRenderInterface final : public Rml::RenderInterface
{
	// definitions
	//-------------
	static Rml::CompiledGeometryHandle const s_InvalidGeometry;
	struct Geometry final
	{
		Geometry() = default;

		render::T_ArrayLoc m_VertexArray = 0u;
		render::T_BufferLoc m_VertexBuffer = 0u;
		render::T_BufferLoc m_IndexBuffer = 0u;

		int32 m_NumVertices = 0;
		int32 m_NumIndices = 0;

		Ptr<render::TextureData const> m_Texture;
	};

	typedef std::unordered_map<Rml::CompiledGeometryHandle, Geometry> T_Geometries;

	static Rml::TextureHandle const s_InvalidTexture;
	struct Texture final
	{
		Texture() = default;
		Texture(AssetPtr<render::TextureData> const& asset) : m_Asset(asset) {}
		Texture(UniquePtr<render::TextureData>&& generated) : m_Generated(std::move(generated)) {}

		Ptr<render::TextureData const> Get() const { return ToPtr((m_Asset != nullptr) ? m_Asset.get() : m_Generated.Get()); }
		
	private:
		AssetPtr<render::TextureData> m_Asset;
		UniquePtr<render::TextureData> m_Generated;
	};

	typedef std::unordered_map<Rml::TextureHandle, Texture> T_Textures;

	// construct destruct
	//--------------------
public:
	RmlRenderInterface();
	~RmlRenderInterface() = default;

	// functionality
	//---------------
	void SetGraphicsContext(Ptr<render::I_GraphicsContextApi> const graphicsContext) { m_GraphicsContext = graphicsContext; }
	void SetShader(AssetPtr<render::ShaderData> const& shader) { m_Shader = shader; }
	void SetView(ivec2 const dim, mat4 const& viewProj) { m_ViewDimensions = dim; m_ViewProj = viewProj; }

	// interface implementation
	//--------------------------

	// doesn't need to be handled because we compile geometry
	void RenderGeometry(Rml::Vertex*, int32, int32*, int32, Rml::TextureHandle, Rml::Vector2f const&) override {} 

	// geometry
	Rml::CompiledGeometryHandle CompileGeometry(Rml::Vertex* vertices,
		int32 numVertices, 
		int32* indices, 
		int32 numIndices, 
		Rml::TextureHandle textureHandle) override;
	void RenderCompiledGeometry(Rml::CompiledGeometryHandle geometry, Rml::Vector2f const& translation) override;
	void ReleaseCompiledGeometry(Rml::CompiledGeometryHandle geometry) override;

	// scissor region
	void EnableScissorRegion(bool enable) override;
	void SetScissorRegion(int32 x, int32 y, int32 width, int32 height) override;

	// texture
	bool LoadTexture(Rml::TextureHandle& textureHandle, Rml::Vector2i& textureDimensions, Rml::String const& source) override;
	bool GenerateTexture(Rml::TextureHandle& textureHandle, Rml::byte const* source, Rml::Vector2i const& sourceDimensions) override;
	void ReleaseTexture(Rml::TextureHandle textureHandle);

	// transform
	void SetTransform(Rml::Matrix4f const* transform) override;

	// utility
	//---------
private:
	UniquePtr<render::TextureData> GenTextureInternal(void const* data, ivec2 dimensions);


	// Data
	///////

	Ptr<render::I_GraphicsContextApi> m_GraphicsContext;
	ivec2 m_ViewDimensions;
	mat4 m_ViewProj;

	AssetPtr<render::ShaderData> m_Shader;
	AssetPtr<render::ShaderData> m_NullShader;

	T_Geometries m_Geometries;
	Rml::CompiledGeometryHandle m_LastGeometryHandle = s_InvalidGeometry;

	T_Textures m_Textures;
	Rml::TextureHandle m_LastTextureHandle = s_InvalidTexture;

	render::TextureParameters m_GeneratedParameters;
	UniquePtr<render::TextureData> m_EmptyWhiteTex2x2;

	bool m_IsScissorEnabled = false;
	ivec2 m_ScissorPos;
	ivec2 m_ScissorSize;

	mat4 m_CurrentTransform;
	bool m_HasTransform = false;

	ivec2 m_LastScissorPos; // to avoid repeating the same stencil rendering for transformed scissor
	ivec2 m_LastScissorSize;
	mat4 m_LastTransform;
};


} // namespace gui
} // namespace et

