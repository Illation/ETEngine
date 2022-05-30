#pragma once
#include <EtCore/Content/AssetPointer.h>

#include <EtGui/Content/SdfFont.h>


namespace et { namespace rt {
	class SplashScreenRenderer;
} namespace render {
	class ShaderData;
} }


namespace et {
namespace gui {


class GuiRenderer;


//---------------------------------
// TextRenderer
//
// Draws sprite fonts in an efficient manner
//
class TextRenderer final 
{
	// definitions 
	//-------------
private:
	friend class rt::SplashScreenRenderer;
	friend class GuiRenderer;

	//---------------------------------
	// TextRenderer::TextVertex
	//
	// Vertex data to be sent ot the GPU
	//
	struct TextVertex
	{
		vec3 Position;
		vec4 Color;
		vec2 TexCoord;
		vec2 CharacterDimension;
		float SizeMult;
		uint32 ChannelId;
	};

	//---------------------------------
	// TextRenderer::TextCache
	//
	// Cached text that will be rendered by the TextRenderer
	//
	struct TextCache
	{
		// construct 
		//-----------
	public:
		TextCache(std::string const& text, vec2 const pos, vec4 const& col, int16 const size);

	private:
		TextCache& operator=(const TextCache &tmp); // disable assignment

		// Data
		///////

	public:
		std::string const Text;
		vec2 Position;
		vec4 Color;
		int16 Size;
	};

	//---------------------------------
	// TextRenderer::QueuedFont
	//
	// Information about how a font will be drawn
	//
	struct QueuedFont
	{
		Ptr<SdfFont const> m_Font;

		int32 m_BufferStart = 0;
		int32 m_BufferSize = 0;
		bool m_IsAddedToRenderer = false;

		std::vector<TextCache> m_TextCache;
	};

	// c-tor d-tor
	//-------------
	TextRenderer() = default;
	~TextRenderer();

	void Initialize();
	void Deinit();

	// functionality
	//---------------
public:
	void SetFont(Ptr<SdfFont const> const font);
	void SetColor(vec4 const& color) { m_Color = color; }
	void DrawText(std::string const& text, vec2 const pos, int16 fontSize = 0);//fontSize 0 means using the fonts default size
	void OnWindowResize();

	// accessors
	//-----------
	ivec2 GetTextSize(std::string const& text, SdfFont const* const font, int16 fontSize = 0) const;

	// utility
	//---------
private:
	void Draw();
	void UpdateBuffer();

	void CalculateTransform();

	// Data
	///////

	bool m_IsInitialized = false;

	bool m_bUseKerning = true;

	AssetPtr<render::ShaderData> m_TextShader;
	std::vector<QueuedFont> m_QueuedFonts;

	uint32 m_BufferSize = 500;
	uint32 m_NumCharacters = 0;
	mat4 m_Transform;
	vec4 m_Color = vec4(0, 0, 0, 1);
	size_t m_ActiveFontIdx;

	render::T_ArrayLoc m_VAO;
	render::T_BufferLoc m_VBO;

	render::T_ViewportEventCallbackId m_VPCallbackId = render::T_ViewportEventDispatcher::INVALID_ID;
};


} // namespace gui
} // namespace et
