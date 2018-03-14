#pragma once

class SpriteFont;
class ShaderData;

class TextRenderer : public Singleton<TextRenderer>
{
public:

#undef DrawText
	void DrawText(std::string &text, vec2 pos, int16 fontSize = 0);//fontSize 0 means using the fonts default size
	void SetFont(SpriteFont* pFont);
	void SetColor(vec4 color) { m_Color = color; }

	ivec2 GetTextSize(const std::string &text, SpriteFont* pFont, int16 fontSize = 0);

	void OnWindowResize();
private:

	friend class Singleton<TextRenderer>;
	friend class AbstractScene;
	friend class AbstractFramework;
	friend class RenderPipeline;
	friend class UIPortal;
#ifdef EDITOR
	friend class Editor;
#endif

	TextRenderer();
	~TextRenderer();

	struct TextVertex
	{
		vec3 Position;
		vec4 Color;
		vec2 TexCoord;
		vec2 CharacterDimension;
		float SizeMult;
		uint32 ChannelId;
	};

	void Initialize();
	void Draw();
	void UpdateBuffer();

	void CalculateTransform();

	bool m_bUseKerning = true;

	ShaderData* m_pTextShader;
	std::vector<SpriteFont*> m_pSpriteFonts;

	uint32 m_BufferSize, m_NumCharacters;
	mat4 m_Transform;
	vec4 m_Color = vec4(0, 0, 0, 1);
	size_t m_ActiveFontIdx;

	GLint m_uTransform;
	GLint m_uTexSize;
	GLint m_uTexture;

	GLuint m_VAO;
	GLuint m_VBO;
};

