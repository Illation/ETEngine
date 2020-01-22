#pragma once
#include <EtCore/Content/AssetPointer.h>


namespace et {
namespace render {


class ShadedSceneRenderer;
class SplashScreenRenderer;


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
	friend class render::SplashScreenRenderer;
	friend class render::ShadedSceneRenderer;
	friend class UIPortal;

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
	void Draw(TextureData const* tex, 
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
	T_ArrayLoc m_VAO = 0;
	T_BufferLoc m_VBO = 0;

	//Textures
	TextureData* m_EmptyTex = nullptr;
	std::vector<TextureData const*> m_Textures;

	//Shader and its uniforms
	AssetPtr<ShaderData> m_Shader;

	mat4 m_Transform;

	float m_Layer;

	render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace render
} // namespace et
