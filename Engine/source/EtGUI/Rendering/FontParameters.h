#pragma once
#include <RmlUi/Core/Vertex.h>


namespace et {
namespace gui {


//---------------------------------
// TextLayer
//
// Parameters for rendering a pass of text
//
struct TextLayer
{
	ivec2 m_Offset;

	vec4 m_Color = vec4(1.f); // ignored on main layer

	// thresholds are offset from the base offset based on the font weight
	float m_SdfThreshold = 0.f; // double as max threshold
	float m_MinThreshold = 0.f; // for blurred text

	bool m_IsBlurred = false;
	bool m_IsMainLayer = false;
};

//----------------------------------------
// FontParameters
//
// Additional parameters for rendering Sdf Font text
//
struct FontParameters
{
	static size_t GetVCount() // required size to store parameters in vertex data
	{
		return (sizeof(FontParameters) + sizeof(Rml::Vertex) - 1) / sizeof(Rml::Vertex);
	}

	float m_SdfThreshold = 0.5f; // base, may be offset by layers
	size_t m_LayerCount = 0u;
	vec4 m_MainLayerColor = vec4(1.f);
	Ptr<TextLayer> m_Layers;
};

//---------------------------------
// TextVertex
//
// Vertex data for text rendering
//
struct TextVertex
{
	vec2 m_Position;
	vec2 m_TexCoord;
	uint8 m_Channel;
};


} // namespace gui
} // namespace et

