#include "stdafx.hpp"
#include "Hash.h"

uint32 FnvHash(const std::string &str)
{
	return detail::fnv1a_32(str.c_str(), str.size());
}
