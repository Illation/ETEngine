#include "stdafx.h"
#include "PostProcessingSettings.h"


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<PostProcessingSettings>("post processing settings")
		.property("exposure", &PostProcessingSettings::exposure)
		.property("gamma", &PostProcessingSettings::gamma)
		.property("bloomThreshold", &PostProcessingSettings::bloomThreshold)
		.property("bloomMult", &PostProcessingSettings::bloomMult);
}

DEFINE_FORCED_LINKING(PostProcessingSettings) // force the linker to include this unit

