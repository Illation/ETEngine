#include "stdafx.h"
#include "PostProcessingSettings.h"


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<PostProcessingSettings>("post processing settings")
		.property("exposure", &PostProcessingSettings::exposure)
		.property("gamma", &PostProcessingSettings::gamma)
		.property("bloom threshold", &PostProcessingSettings::bloomThreshold)
		.property("bloom mult", &PostProcessingSettings::bloomMult);
}

DEFINE_FORCED_LINKING(PostProcessingSettings) // force the linker to include this unit

