#include "stdafx.h"
#include "RmlFontEngineInterface.h"


namespace et {
namespace gui {


//===========================
// RML Font Engine Interface
//===========================


// static
Rml::FontEffectsHandle const RmlFontEngineInterface::s_InvalidEffects = 0u;
Rml::FontFaceHandle const RmlFontEngineInterface::s_InvalidFont = 0u;


//--------------------------------------
// RmlFontEngineInterface::LoadFontFace
//
bool RmlFontEngineInterface::LoadFontFace(Rml::String const& fileName, bool const fallbackFace, Rml::Style::FontWeight const weight)
{
	UNUSED(fileName);
	UNUSED(fallbackFace);
	UNUSED(weight);

	return false;
}

//--------------------------------------
// RmlFontEngineInterface::LoadFontFace
//
bool RmlFontEngineInterface::LoadFontFace(byte const* const data, 
	int32 const dataSize, 
	Rml::String const& family, 
	Rml::Style::FontStyle const style, 
	Rml::Style::FontWeight const weight, 
	bool const fallbackFace)
{
	UNUSED(data);
	UNUSED(dataSize);
	UNUSED(family);
	UNUSED(style);
	UNUSED(weight);
	UNUSED(fallbackFace);

	return false;
}

//-------------------------------------------
// RmlFontEngineInterface::GetFontFaceHandle
//
Rml::FontFaceHandle RmlFontEngineInterface::GetFontFaceHandle(Rml::String const& family, 
	Rml::Style::FontStyle const style, 
	Rml::Style::FontWeight const weight, 
	int32 const size)
{
	UNUSED(family);
	UNUSED(style);
	UNUSED(weight);
	UNUSED(size);

	return s_InvalidFont;
}

//--------------------------------------------
// RmlFontEngineInterface::PrepareFontEffects
//
Rml::FontEffectsHandle RmlFontEngineInterface::PrepareFontEffects(Rml::FontFaceHandle const faceHandle, Rml::FontEffectList const& fontEffects)
{
	UNUSED(faceHandle);
	UNUSED(fontEffects);

	return s_InvalidEffects;
}

//---------------------------------
// RmlFontEngineInterface::GetSize
//
int32 RmlFontEngineInterface::GetSize(Rml::FontFaceHandle const faceHandle)
{
	UNUSED(faceHandle);

	return 0;
}

//------------------------------------
// RmlFontEngineInterface::GetXHeight
//
int32 RmlFontEngineInterface::GetXHeight(Rml::FontFaceHandle const faceHandle)
{
	UNUSED(faceHandle);

	return 0;
}

//---------------------------------------
// RmlFontEngineInterface::GetLineHeight
//
int32 RmlFontEngineInterface::GetLineHeight(Rml::FontFaceHandle const faceHandle)
{
	UNUSED(faceHandle);

	return 0;
}

//-------------------------------------
// RmlFontEngineInterface::GetBaseline
//
int32 RmlFontEngineInterface::GetBaseline(Rml::FontFaceHandle const faceHandle)
{
	UNUSED(faceHandle);

	return 0;
}

//--------------------------------------
// RmlFontEngineInterface::GetUnderline
//
float RmlFontEngineInterface::GetUnderline(Rml::FontFaceHandle const faceHandle, float& outThickness)
{
	UNUSED(faceHandle);

	outThickness = 0.f;
	return 0.f;
}

//----------------------------------------
// RmlFontEngineInterface::GetStringWidth
//
int32 RmlFontEngineInterface::GetStringWidth(Rml::FontFaceHandle const faceHandle, Rml::String const& string, Rml::Character const priorCharacter)
{
	UNUSED(faceHandle);
	UNUSED(string);
	UNUSED(priorCharacter);

	return 0;
}

//----------------------------------------
// RmlFontEngineInterface::GenerateString
//
int32 RmlFontEngineInterface::GenerateString(Rml::FontFaceHandle const faceHandle, 
	Rml::FontEffectsHandle const effectsHandle, 
	Rml::String const& string, 
	Rml::Vector2f const& position, 
	Rml::Colourb const& colour, 
	float const opacity, 
	Rml::GeometryList& outGeometry)
{
	UNUSED(faceHandle);
	UNUSED(effectsHandle);
	UNUSED(string);
	UNUSED(position);
	UNUSED(colour);
	UNUSED(opacity);

	outGeometry = Rml::GeometryList();
	return 0;
}

//------------------------------------
// RmlFontEngineInterface::GetVersion
//
int32 RmlFontEngineInterface::GetVersion(Rml::FontFaceHandle const faceHandle)
{
	UNUSED(faceHandle);

	return 0;
}

//----------------------------------------------
// RmlFontEngineInterface::ReleaseFontResources
//
void RmlFontEngineInterface::ReleaseFontResources()
{

}


} // namespace gui
} // namespace et
