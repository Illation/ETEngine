#pragma once

struct iRect
{
	iRect() = default;
	iRect( ivec2 Pos, ivec2 Size )
		:pos(Pos), size(Size) { }

	ivec2 pos;
	ivec2 size;
};

struct UISprite
{
	iRect rect;
	vec4 color;
	TextureData* texture;
};