#include "stdafx.hpp"
#include "TextRenderer.hpp"

#include "../Graphics/SpriteFont.hpp"
#include "../Graphics/ShaderData.hpp"
#include "../Graphics/TextureData.hpp"

#include <algorithm>

TextRenderer::TextRenderer()
	:m_BufferSize(500)
	,m_Transform(mat4())
	,m_NumCharacters(0)
	,m_pSpriteFonts(std::vector<SpriteFont*>())
{
}

void TextRenderer::Initialize()
{
	m_pTextShader = ContentManager::Load<ShaderData>("Shaders/PostText.glsl");

	STATE->SetShader(m_pTextShader);
	m_uTransform = glGetUniformLocation(m_pTextShader->GetProgram(), "transform");
	m_uTexSize = glGetUniformLocation(m_pTextShader->GetProgram(), "texSize");

	m_uTexture = glGetUniformLocation(m_pTextShader->GetProgram(), "fontTex");
	glUniform1i(m_uTexture, 0);

	//Generate buffers and arrays
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);


	//bind
	STATE->BindVertexArray(m_VAO);
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);

	//set data and attributes
	glBufferData(GL_ARRAY_BUFFER, m_BufferSize, NULL, GL_DYNAMIC_DRAW);

	//input layout

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);

	glVertexAttribPointer(0, (GLint)3, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(TextVertex), (GLvoid*)offsetof(TextVertex, Position));
	glVertexAttribPointer(1, (GLint)4, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(TextVertex), (GLvoid*)offsetof(TextVertex, Color));
	glVertexAttribPointer(2, (GLint)2, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(TextVertex), (GLvoid*)offsetof(TextVertex, TexCoord));
	glVertexAttribPointer(3, (GLint)2, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(TextVertex), (GLvoid*)offsetof(TextVertex, CharacterDimension));
	glVertexAttribPointer(4, (GLint)1, GL_FLOAT, GL_FALSE, (GLsizei)sizeof(TextVertex), (GLvoid*)offsetof(TextVertex, SizeMult));
	glVertexAttribIPointer(5, (GLint)1, GL_UNSIGNED_INT, (GLsizei)sizeof(TextVertex), (GLvoid*)offsetof(TextVertex, ChannelId));

	//unbind
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);
	STATE->BindVertexArray(0);

	CalculateTransform();

	WINDOW.WindowResizeEvent.AddListener( std::bind( &TextRenderer::OnWindowResize, this ) );
}

void TextRenderer::SetFont(SpriteFont* pFont)
{
	auto pos = std::find(m_pSpriteFonts.begin(), m_pSpriteFonts.end(), pFont);
	if (pos == m_pSpriteFonts.end())
	{
		m_ActiveFontIdx = m_pSpriteFonts.size();
		m_pSpriteFonts.push_back(pFont);
	}
	else m_ActiveFontIdx = pos - m_pSpriteFonts.begin();
}

ivec2 TextRenderer::GetTextSize(const std::string &text, SpriteFont* pFont)
{
	vec2 ret = vec2(0);
	for (auto charId : text)
	{
		char previous = 0;
		if (SpriteFont::IsCharValid(charId) && pFont->GetMetric(charId).IsValid)
		{
			auto metric = pFont->GetMetric(charId);

			vec2 kerningVec = 0;
			if (pFont->m_UseKerning)kerningVec = metric.GetKerningVec(static_cast<wchar_t>(previous));
			previous = charId;
			ret.x += (int32)kerningVec.x;

			if (charId == ' ')
			{
				ret.x += metric.AdvanceX;
				continue;
			}

			ret.x += metric.AdvanceX;
			ret.y = std::max(ret.y, (float)metric.Height + (float)metric.OffsetY + kerningVec.y);
		}
		else LOG("TextRenderer::GetTextSize>char not supported for current font", Warning);
	}
	return etm::vecCast<int32>(ret);
}

void TextRenderer::OnWindowResize()
{
	CalculateTransform();
}

void TextRenderer::DrawText(std::string &text, vec2 pos, int16 fontSize)
{
	if (m_pSpriteFonts.size() > 0)
	{
		if (fontSize <= 0)fontSize = m_pSpriteFonts[m_ActiveFontIdx]->GetFontSize();
		m_NumCharacters += (uint32)text.size();
		m_pSpriteFonts[m_ActiveFontIdx]->m_TextCache.push_back(TextCache(text, pos, m_Color, fontSize));
		if (!m_pSpriteFonts[m_ActiveFontIdx]->m_IsAddedToRenderer)
		{
			m_pSpriteFonts[m_ActiveFontIdx]->m_IsAddedToRenderer = true;
		}
	}
	else LOG("TextRenderer>DrawText: No active font found!", Warning);
}

void TextRenderer::Draw()
{
	if (m_pSpriteFonts.size() <= 0) return;

	//Bind Object vertex array
	STATE->BindVertexArray(m_VAO);

	UpdateBuffer();

	if (INPUT->IsKeyboardKeyPressed('K'))m_bUseKerning = !m_bUseKerning;

	//Enable this objects shader
	CalculateTransform();
	STATE->SetShader(m_pTextShader);
	STATE->SetActiveTexture(0);
	glUniformMatrix4fv(m_uTransform, 1, GL_FALSE, etm::valuePtr(m_Transform));

	//Bind Object vertex array
	STATE->BindVertexArray(m_VAO);
	for (auto pFont : m_pSpriteFonts)
	{
		if (pFont->m_IsAddedToRenderer)
		{
			STATE->BindTexture(GL_TEXTURE_2D, pFont->m_pTexture->GetHandle());

			auto texSize = pFont->m_pTexture->GetResolution();
			glUniform2f(m_uTexSize, (float)texSize.x, (float)texSize.y);

			//Draw the object
			STATE->DrawArrays(GL_POINTS, pFont->m_BufferStart, pFont->m_BufferSize);

			pFont->m_IsAddedToRenderer = false;
		}
	}
	//unbind vertex array
	STATE->BindVertexArray(0);
}

void TextRenderer::UpdateBuffer()
{
	std::vector<TextVertex> tVerts;
	for (auto pFont : m_pSpriteFonts)
	{
		if (pFont->m_IsAddedToRenderer)
		{
			pFont->m_BufferStart = (int32)tVerts.size() * (sizeof(TextVertex) / sizeof(float));
			pFont->m_BufferSize = 0;
			for (auto cache : pFont->m_TextCache)
			{
				float sizeMult = (float)cache.Size / (float)pFont->GetFontSize();
				float totalAdvanceX = 0;
				char previous = 0;
				for (auto charId : cache.Text)
				{
					if (SpriteFont::IsCharValid(charId) && pFont->GetMetric(charId).IsValid)
					{
						auto metric = pFont->GetMetric(charId);

						vec2 kerningVec = 0;
						if (pFont->m_UseKerning && m_bUseKerning)kerningVec = metric.GetKerningVec(static_cast<wchar_t>(previous)) * sizeMult;
						previous = charId;
						totalAdvanceX += kerningVec.x;

						if (charId == ' ')
						{
							totalAdvanceX += metric.AdvanceX;
							continue;
						}

						TextVertex vText;
						vText.Position.x = cache.Position.x + (totalAdvanceX + metric.OffsetX)*sizeMult;
						vText.Position.y = cache.Position.y + (kerningVec.y + metric.OffsetY)*sizeMult;
						vText.Position.z = 0;
						vText.Color = cache.Color;
						vText.TexCoord = metric.TexCoord;
						vText.CharacterDimension = vec2(metric.Width, metric.Height);
						vText.SizeMult = sizeMult;
						vText.ChannelId = metric.Channel;

						tVerts.push_back(vText);

						totalAdvanceX += metric.AdvanceX;
					}
					else LOG("TextRenderer::CreateTextVertices>char not supported for current font", Warning);
				}
			}
			pFont->m_BufferSize = (int32)tVerts.size() - pFont->m_BufferStart+1;
			pFont->m_TextCache.clear();
		}
	}

	//Bind Object vertex array
	STATE->BindVertexArray(m_VAO);

	uint32 buffersize = (uint32)tVerts.size() * sizeof(TextVertex);

	//Send the vertex buffer again
	STATE->BindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, buffersize, tVerts.data(), GL_DYNAMIC_DRAW);
	STATE->BindBuffer(GL_ARRAY_BUFFER, 0);

	//Done Modifying
	STATE->BindVertexArray(0);

	m_NumCharacters = 0;
}

void TextRenderer::CalculateTransform()
{
	ivec2 viewPos, viewSize;
	STATE->GetViewport(viewPos, viewSize);
	int32 width = viewSize.x, height = viewSize.y;
	float scaleX = (width > 0) ? 2.f / width : 0;
	float scaleY = (height > 0) ? 2.f / height : 0;

	m_Transform = mat4({
		scaleX,	0,			0,		0,
		0,		-scaleY,	0,		0,
		0,		0,			1,		0,
		-1,		1,			0,		1 });
}

TextRenderer::~TextRenderer()
{
	glDeleteVertexArrays(1, &m_VAO);
	glDeleteBuffers(1, &m_VBO);
}
