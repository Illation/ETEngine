#pragma once
#include <EtCore/Containers/slot_map.h>


namespace et {
namespace render {


//------------------
// Light
//
// All data required to render a light
//
struct Light
{
	vec3 m_Color;
	core::T_SlotId m_NodeId; // extract forward vector from transform for directional lights, or size and position for point lights
};


typedef core::T_SlotId T_LightId;


} // namespace render
} // namespace et
