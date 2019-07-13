#pragma once

struct iRect
{
	iRect() = default;
	iRect( ivec2 Pos, ivec2 Size )
		:pos(Pos), size(Size) { }

	ivec2 pos = ivec2(0);
	ivec2 size = ivec2(1);

	bool Contains(const vec2 &lpos)
	{
		if (lpos.x >= (float)pos.x
		&&	lpos.x <= (float)(pos.x + size.x)
		&&	lpos.y >= (float)pos.y
		&&	lpos.y <= (float)(pos.y + size.y))
			return true;
		return false;
	}
};
