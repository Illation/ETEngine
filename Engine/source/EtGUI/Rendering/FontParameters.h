#pragma once
#include <RmlUi/Core/Vertex.h>


namespace et {
namespace gui {


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

	float m_SdfThreshold = 0.f;
};


} // namespace gui
} // namespace et

