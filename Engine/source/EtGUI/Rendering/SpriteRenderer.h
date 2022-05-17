#pragma once
#include <EtCore/Content/AssetPointer.h>


namespace et { namespace rt {
	class SplashScreenRenderer;
} namespace render {
	class ShaderData;
} }


namespace et {
namespace gui {


class GuiRenderer;


//---------------------------------
// SpriteRenderer
//
// Rendering class that can draw 2D images to the current framebuffer
//
class SpriteRenderer final
{
private:
	// Definitions
	//--------------------------
	friend class rt::SplashScreenRenderer;
	friend class GuiRenderer;

	//---------------------------------
	// SpriteRenderer::SpriteVertex
	//
	// Vertex data for the GPU
	//
	struct SpriteVertex
	{
		uint32 TextureId = 0;
		vec4 TransformData;
		vec4 TransformData2;
		vec4 Color;
	};

public:
	//-------------------------------------
	// SpriteRenderer::E_ScalingMode
	//
	// How to relatively scale the sprites
	//
	enum class E_ScalingMode : uint8
	{
		Screen,
		Pixel,
		Texture,
		TextureAbs // Uses true pixels size, unrelated to global texture scaling
	};

private:
	// construct destruct
	//--------------------
	SpriteRenderer() = default;
	~SpriteRenderer();
	SpriteRenderer(const SpriteRenderer& t);
	SpriteRenderer& operator=(const SpriteRenderer& t);

public:

	void Initialize();

	// Functionality
	//---------------
	void Draw(Ptr<render::TextureData const> tex,
		vec2 const& position, 
		vec4 const& color = vec4(1), 
		vec2 const& pivot = vec2(0), 
		vec2 const& scale = vec2(1), 
		float const rotation = 0.f, 
		float const depth = 0.f, 
		E_ScalingMode const mode = E_ScalingMode::Screen,
		float const layer = 0.f);

	void OnWindowResize();

private:
	void Draw();

	// Utility
	//---------
	void UpdateBuffer();

	void CalculateTransform();

	// Data
	///////

	//Vertices
	std::vector<SpriteVertex> m_Sprites;
	uint32 m_BufferSize = 50;
	render::T_ArrayLoc m_VAO = 0;
	render::T_BufferLoc m_VBO = 0;

	//Textures
	UniquePtr<render::TextureData> m_EmptyTex;
	std::vector<Ptr<render::TextureData const>> m_Textures;

	//Shader and its uniforms
	AssetPtr<render::ShaderData> m_Shader;

	mat4 m_Transform;

	float m_Layer;

	render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace render
} // namespace et
