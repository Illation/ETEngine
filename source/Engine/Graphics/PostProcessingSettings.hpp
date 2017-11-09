#pragma once

struct PostProcessingSettings
{
	//High dynamic range
	float exposure = 1.f;

	//Gamma correction
	float gamma = 2.2f;

	//Bloom
	float bloomThreshold = 10.f;
	float bloomMult = 0.1f;
};
