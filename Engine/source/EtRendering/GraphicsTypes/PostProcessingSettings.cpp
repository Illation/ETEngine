#include "stdafx.h"
#include "PostProcessingSettings.h"


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<PostProcessingSettings>("post processing settings")
		.property("exposure", &PostProcessingSettings::exposure)
		.property("shoulder strength", &PostProcessingSettings::shoulderStrength)
		.property("linear strength", &PostProcessingSettings::linearStrength)
		.property("linear angle", &PostProcessingSettings::linearAngle)
		.property("toe strength", &PostProcessingSettings::toeStrength)
		.property("toe numerator", &PostProcessingSettings::toeNumerator)
		.property("toe denominator", &PostProcessingSettings::toeDenominator)
		.property("linear white", &PostProcessingSettings::linearWhite)
		.property("gamma", &PostProcessingSettings::gamma)
		.property("bloom threshold", &PostProcessingSettings::bloomThreshold)
		.property("bloom mult", &PostProcessingSettings::bloomMult);
}

DEFINE_FORCED_LINKING(PostProcessingSettings) // force the linker to include this unit

