#pragma once

struct iRect
{
	ivec2 pos;
	ivec2 size;
};

struct UISprite
{
	iRect rect;
	vec4 color;
	TextureData* texture;
};