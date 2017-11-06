#pragma once

class SpriteRenderer : public Singleton<SpriteRenderer>
{
public:

	void Draw( TextureData* pTexture, vec2 position, vec4 color = vec4(1), 
			   vec2 pivot = vec2( 0 ), vec2 scale = vec2( 1 ), 
			   float rotation = 0.f, float depth = 0.f );

private:

	struct SpriteVertex
	{
		uint32 TextureId = 0;
		vec4 TransformData;
		vec4 TransformData2;
		vec4 Color;

		const bool Equals( const SpriteVertex& rhs )
		{
			return((TextureId == rhs.TextureId) &&
				(etm::nearEqualsV( TransformData, rhs.TransformData )) &&
				(etm::nearEqualsV( TransformData2, rhs.TransformData2 )) &&
				(etm::nearEqualsV( Color, rhs.Color )));
		}
	};

	friend class Singleton<SpriteRenderer>;
	friend class AbstractScene;

	SpriteRenderer();
	~SpriteRenderer();

	void Initialize();
	void Draw();
	void DrawImmediate( TextureData* pSrv, vec2 position, vec4 color, vec2 pivot, vec2 scale, float rotation );
	void UpdateBuffer();

	ShaderData* m_pShader = nullptr;

	vector<SpriteVertex> m_Sprites;
	uint32 m_BufferSize = 50;
	GLuint m_VAO = 0;
	GLuint m_VBO = 0;

	GLint m_uTransform = 0;
	mat4 m_Transform;

	vector<TextureData*> m_Textures;
	GLint m_uTextureSize = 0;
	GLint m_uTexture;

	GLuint m_ImmediateVAO = 0;
	GLuint m_ImmediateVBO = 0;
	SpriteVertex m_ImmediateVertex;

private:
	void CreateVertexBuffer();
	// -------------------------
	// Disabling default copy constructor and default 
	// assignment operator.
	// -------------------------
	SpriteRenderer( const SpriteRenderer& t );
	SpriteRenderer& operator=( const SpriteRenderer& t );
};