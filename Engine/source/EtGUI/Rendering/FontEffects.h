#pragma once
#include <RmlUi/Core/FontEffect.h>

#include "FontParameters.h"

#include <EtGUI/Content/SdfFont.h>


namespace et {
	REGISTRATION_NS(gui);
}


namespace et {
namespace gui {


//---------------------------------
// FontEffectBase
//
// All effects supported by the SDF font engine should inherit from this
//
class FontEffectBase : public Rml::FontEffect
{
	// definitions
	//-------------
	RTTR_ENABLE()
	REGISTRATION_FRIEND_NS(gui)

	// construct destruct
	//--------------------
public:
	FontEffectBase() : Rml::FontEffect() {}
	virtual ~FontEffectBase() = default;

	// interface
	//-----------
	virtual void GetTextLayer(SdfFont const* const font, float const sizeMultiplier, TextLayer& outLayer) const = 0;
};


} // namespace gui
} // namespace et

