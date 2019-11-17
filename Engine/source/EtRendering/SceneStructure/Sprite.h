#pragma once
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Containers/slot_map.h>

class TextureData;


namespace render {


//----------------------
// Sprite
//
// Data required to draw a sprite in a scene
//
struct Sprite
{
	core::T_SlotId node = core::INVALID_SLOT_ID;
	vec2 pivot;

	vec4 color;
	AssetPtr<TextureData> texture;
};


} // namespace render
