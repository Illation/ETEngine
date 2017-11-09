#pragma once

class SpriteRenderer : public Singleton<SpriteRenderer>
{
public:

	void Draw( TextureData* pTexture, vec2 position, vec4 color = vec4(1), 
			   vec2 pivot = vec2( 0 ), vec2 scale = vec2( 1 ), 
			   float rotation = 0.f, float depth = 0.f );

	void OnWindowResize();

private:
	friend class Singleton<SpriteRenderer>;
	friend class RenderPipeline;
#ifdef EDITOR
	friend class EditorRenderer;
#endif

	struct SpriteVertex
	{
		uint32 TextureId = 0;
		vec4 TransformData;
		vec4 TransformData2;
		vec4 Color;
	};

	void Initialize();
	void UpdateBuffer();

	void Draw();

	void CalculateTransform();

	//Vertices
	vector<SpriteVertex> m_Sprites;
	uint32 m_BufferSize = 50;
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;

	//Textures
	vector<TextureData*> m_Textures;

	//Shader and its uniforms
	ShaderData* m_pShader = nullptr;

	GLint m_uTransform = 0;
	mat4 m_Transform;

	GLint m_uTextureSize = 0;
	GLint m_uTexture;

	//Direct drawing
	//void DrawImmediate( TextureData* pSrv, vec2 position, vec4 color, vec2 pivot, vec2 scale, float rotation );
	//GLuint m_ImmediateVAO = 0;
	//GLuint m_ImmediateVBO = 0;
	//SpriteVertex m_ImmediateVertex;

private:
	//Private constructor and destructor for singleton
	SpriteRenderer();
	~SpriteRenderer();
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpriteRenderer( const SpriteRenderer& t );
	SpriteRenderer& operator=( const SpriteRenderer& t );
};