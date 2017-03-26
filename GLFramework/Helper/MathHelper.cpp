#pragma once
#include "stdafx.hpp"
#include "MathHelper.hpp"

string to_string(vec3 in)
{
	return to_string(in.x) + ", " + to_string(in.y) + ", " + to_string(in.z);
}