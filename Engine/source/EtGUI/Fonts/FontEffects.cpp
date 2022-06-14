#include "stdafx.h"
#include "FontEffects.h"

#include <RmlUi/Core/PropertyDefinition.h>


namespace et {
namespace gui {


//==================
// Font Effect Base
//==================


//--------------------------
// FontEffectBase::GetColor
//
void FontEffectBase::GetColor(vec4& outColor) const
{
	Rml::Colourb const col = GetColour();
	outColor.r = static_cast<float>(col.red) / 255.f;
	outColor.g = static_cast<float>(col.green) / 255.f;
	outColor.b = static_cast<float>(col.blue) / 255.f;
	outColor.a = static_cast<float>(col.alpha) / 255.f;
}


//===================
// Font Effects Glow
//===================


//----------------------------------
// FontEffectGlow::Instancer::Init
//
void FontEffectGlow::Instancer::Init()
{
	m_IdWidthOutline = RegisterProperty("width-outline", "1px", true).AddParser("length").GetId();
	m_IdWidthBlur = RegisterProperty("width-blur", "-1px", true).AddParser("length").GetId();
	m_IdOffsetX = RegisterProperty("offset-x", "0px", true).AddParser("length").GetId();
	m_IdOffsetY = RegisterProperty("offset-y", "0px", true).AddParser("length").GetId();
	m_IdColor = RegisterProperty("color", "white", false).AddParser("color").GetId();
	RegisterShorthand("font-effect", "width-outline, width-blur, offset-x, offset-y, color", Rml::ShorthandType::FallThrough);
}

//-----------------------------------------------
// FontEffectGlow::Instancer::InstanceFontEffect
//
// Generate a glow effect
//
std::shared_ptr<Rml::FontEffect> FontEffectGlow::Instancer::InstanceFontEffect(Rml::String const& name, Rml::PropertyDictionary const& properties)
{
	UNUSED(name);

	int32 const widthOutline = properties.GetProperty(m_IdWidthOutline)->Get<int32>();
	int32 widthBlur = properties.GetProperty(m_IdWidthBlur)->Get<int32>();
	ivec2 offset;
	offset.x = properties.GetProperty(m_IdOffsetX)->Get<int32>();
	offset.y = properties.GetProperty(m_IdOffsetY)->Get<int32>();
	Rml::Colourb const color = properties.GetProperty(m_IdColor)->Get<Rml::Colourb>();

	if (widthBlur < 0)
	{
		widthBlur = widthOutline;
	}

	std::shared_ptr<FontEffectGlow> const fontEffect = std::make_shared<FontEffectGlow>();
	if (fontEffect->Init(widthOutline, widthBlur, offset))
	{
		fontEffect->SetColour(color);
		return fontEffect;
	}

	return nullptr;
}

//-----------------------
// FontEffectGlow::c-tor
//
FontEffectGlow::FontEffectGlow() 
	: FontEffectBase()
{
	SetLayer(Rml::FontEffect::Layer::Back);
}

//----------------------
// FontEffectGlow::Init
//
bool FontEffectGlow::Init(int32 const widthOutline, int32 const widthBlur, ivec2 const offset)
{
	if ((widthOutline < 0) || (widthBlur < 0))
	{
		return false;
	}

	m_WidthOutline = widthOutline;
	m_WidthTotal = widthBlur + widthOutline;
	m_Offset = offset;

	return true;
}

//----------------------------------
// FontEffectGlow::PrepareTextLayer
//
void FontEffectGlow::PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const
{
	outLayer.m_Offset = math::vecCast<float>(m_Offset);
	GetColor(outLayer.m_Color);

	outLayer.m_SdfThreshold = static_cast<float>(m_WidthOutline) * -faceMultiplier;
	if (m_WidthTotal == m_WidthOutline) // basically this is now a shadow / outline effect so we don't need to blur it and can use antialiasing
	{
		outLayer.m_IsBlurred = false;
	}
	else
	{
		outLayer.m_MinThreshold = static_cast<float>(m_WidthTotal) * -faceMultiplier;
		outLayer.m_IsBlurred = true;
	}
}


//======================
// Font Effects Outline
//======================


//------------------------------------
// FontEffectOutline::Instancer::Init
//
void FontEffectOutline::Instancer::Init()
{
	m_IdWidth = RegisterProperty("width", "1px", true).AddParser("length").GetId();
	m_IdColor = RegisterProperty("color", "white", false).AddParser("color").GetId();
	RegisterShorthand("font-effect", "width, color", Rml::ShorthandType::FallThrough);
}

//--------------------------------------------------
// FontEffectOutline::Instancer::InstanceFontEffect
//
// Generate a glow effect
//
std::shared_ptr<Rml::FontEffect> FontEffectOutline::Instancer::InstanceFontEffect(Rml::String const& name, Rml::PropertyDictionary const& properties)
{
	UNUSED(name);

	float const width= properties.GetProperty(m_IdWidth)->Get<float>();
	Rml::Colourb const color = properties.GetProperty(m_IdColor)->Get<Rml::Colourb>();

	std::shared_ptr<FontEffectOutline> const fontEffect = std::make_shared<FontEffectOutline>();
	if (fontEffect->Init(width))
	{
		fontEffect->SetColour(color);
		return fontEffect;
	}

	return nullptr;
}

//--------------------------
// FontEffectOutline::c-tor
//
FontEffectOutline::FontEffectOutline()
	: FontEffectBase()
{
	SetLayer(Rml::FontEffect::Layer::Back);
}

//-------------------------
// FontEffectOutline::Init
//
bool FontEffectOutline::Init(float const width)
{
	if (width <= 0.f)
	{
		return false;
	}

	m_Width = width;
	return true;
}

//-------------------------------------
// FontEffectOutline::PrepareTextLayer
//
void FontEffectOutline::PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const
{
	outLayer.m_Offset = vec2(0.f);
	GetColor(outLayer.m_Color);

	outLayer.m_SdfThreshold = m_Width * -faceMultiplier;
	outLayer.m_IsBlurred = false;
}


//=====================
// Font Effects Shadow
//=====================


//-----------------------------------
// FontEffectShadow::Instancer::Init
//
void FontEffectShadow::Instancer::Init()
{
	m_IdOffsetX = RegisterProperty("offset-x", "0px", true).AddParser("length").GetId();
	m_IdOffsetY = RegisterProperty("offset-y", "0px", true).AddParser("length").GetId();
	m_IdColor = RegisterProperty("color", "white", false).AddParser("color").GetId();
	RegisterShorthand("offset", "offset-x, offset-y", Rml::ShorthandType::FallThrough);
	RegisterShorthand("font-effect", "offset-x, offset-y, color", Rml::ShorthandType::FallThrough);
}

//-------------------------------------------------
// FontEffectShadow::Instancer::InstanceFontEffect
//
// Generate a glow effect
//
std::shared_ptr<Rml::FontEffect> FontEffectShadow::Instancer::InstanceFontEffect(Rml::String const& name, Rml::PropertyDictionary const& properties)
{
	UNUSED(name);

	ivec2 offset;
	offset.x = properties.GetProperty(m_IdOffsetX)->Get<int32>();
	offset.y = properties.GetProperty(m_IdOffsetY)->Get<int32>();
	Rml::Colourb const color = properties.GetProperty(m_IdColor)->Get<Rml::Colourb>();

	std::shared_ptr<FontEffectShadow> const fontEffect = std::make_shared<FontEffectShadow>();
	if (fontEffect->Init(offset))
	{
		fontEffect->SetColour(color);
		return fontEffect;
	}

	return nullptr;
}

//-------------------------
// FontEffectShadow::c-tor
//
FontEffectShadow::FontEffectShadow()
	: FontEffectBase()
{
	SetLayer(Rml::FontEffect::Layer::Back);
}

//------------------------
// FontEffectShadow::Init
//
bool FontEffectShadow::Init(ivec2 const offset)
{
	m_Offset = offset;
	return true;
}

//------------------------------------
// FontEffectShadow::PrepareTextLayer
//
void FontEffectShadow::PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const
{
	UNUSED(faceMultiplier);

	outLayer.m_Offset = math::vecCast<float>(m_Offset);
	GetColor(outLayer.m_Color);

	outLayer.m_SdfThreshold = 0.f;
	outLayer.m_IsBlurred = false;
}


//===================
// Font Effects Blur
//===================


//---------------------------------
// FontEffectBlur::Instancer::Init
//
void FontEffectBlur::Instancer::Init()
{
	m_IdWidth = RegisterProperty("width", "1px", true).AddParser("length").GetId();
	m_IdColor = RegisterProperty("color", "white", false).AddParser("color").GetId();
	RegisterShorthand("font-effect", "width, color", Rml::ShorthandType::FallThrough);
}

//-----------------------------------------------
// FontEffectBlur::Instancer::InstanceFontEffect
//
// Generate a glow effect
//
std::shared_ptr<Rml::FontEffect> FontEffectBlur::Instancer::InstanceFontEffect(Rml::String const& name, Rml::PropertyDictionary const& properties)
{
	UNUSED(name);

	float const width = properties.GetProperty(m_IdWidth)->Get<float>();
	Rml::Colourb const color = properties.GetProperty(m_IdColor)->Get<Rml::Colourb>();

	std::shared_ptr<FontEffectBlur> const fontEffect = std::make_shared<FontEffectBlur>();
	if (fontEffect->Init(width))
	{
		fontEffect->SetColour(color);
		return fontEffect;
	}

	return nullptr;
}

//-----------------------
// FontEffectBlur::c-tor
//
FontEffectBlur::FontEffectBlur()
	: FontEffectBase()
{
	SetLayer(Rml::FontEffect::Layer::Back);
}

//----------------------
// FontEffectBlur::Init
//
bool FontEffectBlur::Init(float const width)
{
	if (width <= 0.f)
	{
		return false;
	}

	m_Width = width;
	return true;
}

//----------------------------------
// FontEffectBlur::PrepareTextLayer
//
void FontEffectBlur::PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const
{
	outLayer.m_Offset = vec2(0.f);
	GetColor(outLayer.m_Color);

	float const multiplier = faceMultiplier * 0.5f;

	outLayer.m_SdfThreshold = m_Width * multiplier;
	outLayer.m_MinThreshold = m_Width * -multiplier;
	outLayer.m_IsBlurred = true;
}


} // namespace gui
} // namespace et
