#pragma once


namespace et {
namespace render {


//---------------------------------
// Config::Settings::Graphics
//
// Configuration for our rt audio output stream
//
struct GraphicsSettings
{
	bool UseFXAA = true;

	//Shadow Quality
	int32 NumCascades = 3;
	float CSMDrawDistance = 200.f;
	int32 NumPCFSamples = 3;

	int32 PbrBrdfLutSize = 512;

	float TextureScaleFactor = 1.f; // currently not used

	//Bloom Quality
	int32 NumBlurPasses = 5;
};


} // namespace render
} // namespace et
