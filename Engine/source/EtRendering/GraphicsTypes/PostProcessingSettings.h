#pragma once
#include <rttr/type>

#include <EtCore/Helper/LinkerUtils.h>


//---------------------------------
// PostProcessingSettings
//
// Defines tuning for post processing
//
struct PostProcessingSettings final
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE()

public:
	//High dynamic range
	float exposure = 1.f;

	//Gamma correction
	float gamma = 2.2f;

	//Bloom
	float bloomThreshold = 10.f;
	float bloomMult = 0.1f;
};
