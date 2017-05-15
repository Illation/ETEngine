#pragma once

class SpriteFont;
class ShaderData;

class TextRenderer : public Singleton<TextRenderer>
{
public:

#undef DrawText
	void DrawText(std::string &text, glm::vec2 pos);
	void SetFont(SpriteFont* pFont);
	void SetColor(glm::vec4 color) { m_Color = color; }

private:

	friend class Singleton<TextRenderer>;
	friend class AbstractScene;
	friend class AbstractFramework;
	friend class RenderPipeline;

	TextRenderer();
	~TextRenderer();

	struct TextVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		glm::vec2 CharacterDimension;
		uint32 ChannelId;
	};

	void Initialize();
	void Draw();
	void UpdateBuffer();

	ShaderData* m_pTextShader;
	std::vector<SpriteFont*> m_pSpriteFonts;

	uint32 m_BufferSize, m_NumCharacters;
	glm::mat4 m_Transform;
	glm::vec4 m_Color = glm::vec4(0, 0, 0, 1);
	size_t m_ActiveFontIdx;

	GLint m_uTransform;
	GLint m_uTexSize;
	GLint m_uTexture;

	GLuint m_VAO;
	GLuint m_VBO;
};

