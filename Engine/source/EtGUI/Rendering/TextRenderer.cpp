#include "stdafx.h"
#include "TextRenderer.h"

#include <EtCore/Content/ResourceManager.h>

#include <EtRendering/GraphicsTypes/Shader.h>
#include <EtRendering/GraphicsTypes/TextureData.h>


namespace et {
namespace gui {


//=============================
// Text Renderer :: Text Cache
//=============================


//---------------------------------
// TextRenderer::TextCache::c-tor
//
// Initialize a text cache
//
TextRenderer::TextCache::TextCache(std::string const& text, vec2 const pos, vec4 const& col, int16 const size)
	: Text(text)
	, Position(pos)
	, Color(col)
	, Size(size)
{ }


//===============
// Text Renderer
//===============


//---------------------------------
// TextRenderer::d-tor
//
// Delete GPU objects
//
TextRenderer::~TextRenderer()
{
	if (m_IsInitialized)
	{
		Deinit();
	}
}

//---------------------------------
// TextRenderer::Initialize
//
// Create handles to GPU objects
//
void TextRenderer::Initialize()
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	m_TextShader = core::ResourceManager::Instance()->GetAssetData<render::ShaderData>(core::HashString("Shaders/PostText.glsl"));

	//Generate buffers and arrays
	m_VAO = api->CreateVertexArray();
	m_VBO = api->CreateBuffer();


	//bind
	api->BindVertexArray(m_VAO);
	api->BindBuffer(render::E_BufferType::Vertex, m_VBO);

	//set data and attributes
	api->SetBufferData(render::E_BufferType::Vertex, m_BufferSize, nullptr, render::E_UsageHint::Dynamic);

	//input layout

	api->SetVertexAttributeArrayEnabled(0, true);
	api->SetVertexAttributeArrayEnabled(1, true);
	api->SetVertexAttributeArrayEnabled(2, true);
	api->SetVertexAttributeArrayEnabled(3, true);
	api->SetVertexAttributeArrayEnabled(4, true);
	api->SetVertexAttributeArrayEnabled(5, true);

	int32 const vertSize = sizeof(TextVertex);
	api->DefineVertexAttributePointer(0, 3, render::E_DataType::Float, false, vertSize, offsetof(TextVertex, Position));
	api->DefineVertexAttributePointer(1, 4, render::E_DataType::Float, false, vertSize, offsetof(TextVertex, Color));
	api->DefineVertexAttributePointer(2, 2, render::E_DataType::Float, false, vertSize, offsetof(TextVertex, TexCoord));
	api->DefineVertexAttributePointer(3, 2, render::E_DataType::Float, false, vertSize, offsetof(TextVertex, CharacterDimension));
	api->DefineVertexAttributePointer(4, 1, render::E_DataType::Float, false, vertSize, offsetof(TextVertex, SizeMult));
	api->DefineVertexAttribIPointer(5, 1, render::E_DataType::UInt, vertSize, offsetof(TextVertex, ChannelId));

	//unbind
	api->BindBuffer(render::E_BufferType::Vertex, 0);
	api->BindVertexArray(0);

	CalculateTransform();

	m_VPCallbackId = render::Viewport::GetCurrentViewport()->GetEventDispatcher().Register(
		render::E_ViewportEvent::VP_Resized, render::T_ViewportEventCallback(
		[this](render::T_ViewportEventFlags const, render::ViewportEventData const* const) -> void
		{
			OnWindowResize();
		}));

	m_IsInitialized = true;
}

//------------------------
// TextRenderer::Deinit
//
void TextRenderer::Deinit()
{
	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	m_TextShader = nullptr;

	if (m_VPCallbackId != render::T_ViewportEventDispatcher::INVALID_ID)
	{
		render::Viewport::GetCurrentViewport()->GetEventDispatcher().Unregister(m_VPCallbackId);
	}

	api->DeleteVertexArray(m_VAO);
	api->DeleteBuffer(m_VBO);

	m_IsInitialized = false;
}

//---------------------------------
// TextRenderer::SetFont
//
// Sets the active font and adds it to the queue if it's not there yet
//
void TextRenderer::SetFont(Ptr<SdfFont const> const font)
{
	auto foundIt = std::find_if(m_QueuedFonts.begin(), m_QueuedFonts.end(), [font](QueuedFont const& queued)
		{
			return queued.m_Font == font;
		});

	if (foundIt == m_QueuedFonts.cend())
	{
		m_ActiveFontIdx = m_QueuedFonts.size();
		m_QueuedFonts.emplace_back(QueuedFont());
		m_QueuedFonts[m_ActiveFontIdx].m_Font = font;
	}
	else
	{
		m_ActiveFontIdx = foundIt - m_QueuedFonts.begin();
	}
}

//---------------------------------
// TextRenderer::DrawText
//
// Creates a text cache and adds it to the active font
//
void TextRenderer::DrawText(std::string const& text, vec2 const pos, int16 fontSize)
{
	ET_ASSERT(m_QueuedFonts.size() > 0u, "No active font set!");

	if (fontSize <= 0)
	{
		fontSize = m_QueuedFonts[m_ActiveFontIdx].m_Font->GetFontSize();
	}

	m_NumCharacters += static_cast<uint32>(text.size());
	m_QueuedFonts[m_ActiveFontIdx].m_TextCache.emplace_back(TextCache(text, pos, m_Color, fontSize));
	m_QueuedFonts[m_ActiveFontIdx].m_IsAddedToRenderer = true;
}

//---------------------------------
// TextRenderer::OnWindowResize
//
// Makes sure text size stays consistent when the window size changes
//
void TextRenderer::OnWindowResize()
{
	CalculateTransform();
}

//---------------------------------
// TextRenderer::GetTextSize
//
// Returns the theoretical dimensions of a text that would be drawn
//
ivec2 TextRenderer::GetTextSize(std::string const& text, SdfFont const* const font, int16 fontSize) const
{
	if (fontSize <= 0)
	{
		fontSize = font->GetFontSize();
	}

	float sizeMult = (float)fontSize / (float)font->GetFontSize();
	vec2 ret(0.f);

	char32 previous = 0;
	for (char32 const charId : text)
	{
		SdfFont::Metric const* const metric = font->GetValidMetric(charId);
		if (metric != nullptr)
		{
			vec2 kerningVec = 0;
			if (font->UseKerning())
			{
				kerningVec = metric->GetKerningVec(static_cast<wchar_t>(previous));
			}

			previous = charId;

			ret.x += (metric->m_AdvanceX + kerningVec.x) * sizeMult;

			if (charId == ' ')
			{
				continue;
			}

			ret.y = std::max(ret.y, (static_cast<float>(metric->m_Height) + static_cast<float>(metric->m_OffsetY) + kerningVec.y) * sizeMult);
		}
		else
		{
			LOG("TextRenderer::GetTextSize>char not supported for current font", core::LogLevel::Warning);
		}
	}

	return math::vecCast<int32>(ret);
}

//---------------------------------
// TextRenderer::Draw
//
// Draws all currently queued fonts to the active render target
//
void TextRenderer::Draw()
{
	if (m_QueuedFonts.size() <= 0)
	{
		return;
	}

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	//Bind Object vertex array
	api->BindVertexArray(m_VAO);

	UpdateBuffer();

	//Enable this objects shader
	CalculateTransform();
	api->SetShader(m_TextShader.get());
	m_TextShader->Upload("transform"_hash, m_Transform);

	//Bind Object vertex array
	api->BindVertexArray(m_VAO);

	for (QueuedFont& queued : m_QueuedFonts)
	{
		if (queued.m_IsAddedToRenderer)
		{
			render::TextureData const* const fontTex = queued.m_Font->GetAtlas();
			m_TextShader->Upload("fontTex"_hash, fontTex);
			m_TextShader->Upload("texSize"_hash, math::vecCast<float>(fontTex->GetResolution())); // #todo: possibly we can just query this in glsl

			//Draw the object
			api->DrawArrays(render::E_DrawMode::Points, queued.m_BufferStart, queued.m_BufferSize);

			queued.m_IsAddedToRenderer = false;
		}
	}

	//unbind vertex array
	api->BindVertexArray(0);
}

//---------------------------------
// TextRenderer::UpdateBuffer
//
// Updates the vertex buffer containing font vertices
//
void TextRenderer::UpdateBuffer()
{
	std::vector<TextVertex> tVerts;

	for (QueuedFont& queued : m_QueuedFonts)
	{
		if (queued.m_IsAddedToRenderer)
		{
			queued.m_BufferStart = static_cast<int32>(tVerts.size() * (sizeof(TextVertex) / sizeof(float)));
			queued.m_BufferSize = 0;

			for (TextCache const& cache : queued.m_TextCache)
			{
				float const sizeMult = static_cast<float>(cache.Size) / static_cast<float>(queued.m_Font->GetFontSize());

				float totalAdvanceX = 0.f;
				char32 previous = 0;
				for (char32 const charId : cache.Text)
				{
					SdfFont::Metric const* const metric = queued.m_Font->GetValidMetric(charId);
					if (metric == nullptr)
					{
						LOG(FS("TextRenderer::UpdateBuffer > char '%c' doesn't have a valid metric", charId), core::LogLevel::Warning);
						continue;
					}

					vec2 kerningVec = 0;
					if (queued.m_Font->UseKerning() && m_bUseKerning)
					{
						kerningVec = metric->GetKerningVec(static_cast<wchar_t>(previous)) * sizeMult;
					}

					previous = charId;

					totalAdvanceX += kerningVec.x;

					if (charId == ' ')
					{
						totalAdvanceX += metric->m_AdvanceX;
						continue;
					}

					tVerts.push_back(TextVertex());
					TextVertex& vText = tVerts[tVerts.size()-1];

					vText.Position.x = cache.Position.x + (totalAdvanceX + metric->m_OffsetX)*sizeMult;
					vText.Position.y = cache.Position.y + (kerningVec.y + metric->m_OffsetY)*sizeMult;
					vText.Position.z = 0;
					vText.Color = cache.Color;
					vText.TexCoord = metric->m_TexCoord;
					vText.CharacterDimension = vec2(metric->m_Width, metric->m_Height);
					vText.SizeMult = sizeMult;
					vText.ChannelId = metric->m_Channel;

					totalAdvanceX += metric->m_AdvanceX;
				}
			}

			queued.m_BufferSize = static_cast<int32>(tVerts.size()) * (sizeof(TextVertex) / sizeof(float)) - queued.m_BufferStart;
			queued.m_TextCache.clear();
		}
	}

	render::I_GraphicsContextApi* const api = render::ContextHolder::GetRenderContext();

	//Bind Object vertex array
	api->BindVertexArray(m_VAO);

	//Send the vertex buffer again
	api->BindBuffer(render::E_BufferType::Vertex, m_VBO);
	api->SetBufferData(render::E_BufferType::Vertex, static_cast<uint32>(tVerts.size() * sizeof(TextVertex)), tVerts.data(), render::E_UsageHint::Dynamic);
	api->BindBuffer(render::E_BufferType::Vertex, 0);

	//Done Modifying
	api->BindVertexArray(0);

	m_NumCharacters = 0;
}

//---------------------------------
// TextRenderer::CalculateTransform
//
// Calculates the transformation matrix to be used by the GPU to position sprites
//
void TextRenderer::CalculateTransform()
{
	ivec2 viewPos, viewSize;
	render::ContextHolder::GetRenderContext()->GetViewport(viewPos, viewSize);
	int32 width = viewSize.x, height = viewSize.y;
	float scaleX = (width > 0) ? 2.f / width : 0;
	float scaleY = (height > 0) ? 2.f / height : 0;

	m_Transform = mat4({
		scaleX,	0,			0,		0,
		0,		-scaleY,	0,		0,
		0,		0,			1,		0,
		-1,		1,			0,		1 });
}


} // namespace gui
} // namespace et