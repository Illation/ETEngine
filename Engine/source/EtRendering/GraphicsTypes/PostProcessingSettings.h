#pragma once
#include <rttr/type>

#include <EtCore/Util/LinkerUtils.h>


namespace et {
namespace render {


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
	// High dynamic range
	float exposure = 1.f;

	// filmic tonemapping
	float shoulderStrength = 0.22f;
	float linearStrength = 0.3f;
	float linearAngle = 0.1f;
	float toeStrength = 0.2f;
	float toeNumerator = 0.01f;
	float toeDenominator = 0.3f;

	float linearWhite = 11.2f;

	// Gamma correction
	float gamma = 2.2f;

	//Bloom
	float bloomThreshold = 10.f;
	float bloomMult = 0.1f;
};


} // namespace render
} // namespace et
