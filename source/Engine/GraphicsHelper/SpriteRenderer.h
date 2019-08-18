#pragma once
#include <EtCore/Content/AssetPointer.h>


//---------------------------------
// SpriteRenderer
//
// Rendering class that can draw 2D images to the current framebuffer
//
class SpriteRenderer : public Singleton<SpriteRenderer>
{
private:
	// Definitions
	//--------------------------
	friend class RenderPipeline;
	friend class UIPortal;
#ifdef EDITOR
	friend class Editor;
#endif
	friend class Singleton<SpriteRenderer>;

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
	virtual ~SpriteRenderer();
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
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;

	//Textures
	TextureData* m_EmptyTex = nullptr;
	std::vector<TextureData const*> m_Textures;

	//Shader and its uniforms
	AssetPtr<ShaderData> m_Shader;

	GLint m_uTransform = 0;
	mat4 m_Transform;

	GLint m_uTexture;
	GLint m_u3DTexture;

	GLint m_uDraw3D;
	GLint m_uLayer;
	float m_Layer;
};