#pragma once
#include <RmlUi/Core/FontEffect.h>
#include <RmlUi/Core/FontEffectInstancer.h>

#include "FontParameters.h"


// fwd
namespace et {
	REGISTRATION_NS(gui);
}


namespace et {
namespace gui {


//---------------------------------
// FontEffectBase
//
// Reimplementation of the Rml version of the font effect to make it work with SDF fonts
// All effects supported by the SDF font engine should inherit from this
//
class FontEffectBase : public Rml::FontEffect
{
	// construct destruct
	//--------------------
public:
	FontEffectBase() : Rml::FontEffect() {}
	virtual ~FontEffectBase() = default;

	// interface
	//-----------
	virtual void PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const = 0;

	// utility
	//---------
protected:
	void GetColor(vec4& outColor) const;
};


////////////////////////////////////////////
////////////////////////////////////////////


//---------------------------------
// FontEffectGlow
//
class FontEffectGlow : public FontEffectBase
{
public:
	//---------------------------------
	// Instancer
	//
	// Factory for glow effects
	//
	class Instancer : public Rml::FontEffectInstancer
	{
	public:
		void Init();
		std::shared_ptr<Rml::FontEffect> InstanceFontEffect(Rml::String const& name, Rml::PropertyDictionary const& properties) override;

		// Data
		///////
	private:
		Rml::PropertyId m_IdWidthOutline;
		Rml::PropertyId m_IdWidthBlur;
		Rml::PropertyId m_IdOffsetX;
		Rml::PropertyId m_IdOffsetY;
		Rml::PropertyId m_IdColor;
	};

	// construct destruct
	//--------------------
	FontEffectGlow();
	bool Init(int32 const widthOutline, int32 const widthBlur, ivec2 const offset);

	// interface
	//-----------
	void PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const override;

	// Data
	///////
private:
	int32 m_WidthOutline = 0;
	int32 m_WidthTotal = 0;
	ivec2 m_Offset;
};

//---------------------------------
// FontEffectOutline
//
class FontEffectOutline : public FontEffectBase
{
public:
	//---------------------------------
	// Instancer
	//
	// Factory for glow effects
	//
	class Instancer : public Rml::FontEffectInstancer
	{
	public:
		void Init();
		std::shared_ptr<Rml::FontEffect> InstanceFontEffect(Rml::String const& name, Rml::PropertyDictionary const& properties) override;

		// Data
		///////
	private:
		Rml::PropertyId m_IdWidth;
		Rml::PropertyId m_IdColor;
	};

	// construct destruct
	//--------------------
	FontEffectOutline();
	bool Init(float const width);

	// interface
	//-----------
	void PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const override;

	// Data
	///////
private:
	float m_Width = 0;
};

//---------------------------------
// FontEffectShadow
//
class FontEffectShadow : public FontEffectBase
{
public:
	//---------------------------------
	// Instancer
	//
	// Factory for glow effects
	//
	class Instancer : public Rml::FontEffectInstancer
	{
	public:
		void Init();
		std::shared_ptr<Rml::FontEffect> InstanceFontEffect(Rml::String const& name, Rml::PropertyDictionary const& properties) override;

		// Data
		///////
	private:
		Rml::PropertyId m_IdOffsetX;
		Rml::PropertyId m_IdOffsetY;
		Rml::PropertyId m_IdColor;
	};

	// construct destruct
	//--------------------
	FontEffectShadow();
	bool Init(ivec2 const offset);

	// interface
	//-----------
	void PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const override;

	// Data
	///////
private:
	ivec2 m_Offset;
};

//---------------------------------
// FontEffectBlur
//
class FontEffectBlur : public FontEffectBase
{
public:
	//---------------------------------
	// Instancer
	//
	// Factory for glow effects
	//
	class Instancer : public Rml::FontEffectInstancer
	{
	public:
		void Init();
		std::shared_ptr<Rml::FontEffect> InstanceFontEffect(Rml::String const& name, Rml::PropertyDictionary const& properties) override;

		// Data
		///////
	private:
		Rml::PropertyId m_IdWidth;
		Rml::PropertyId m_IdColor;
	};

	// construct destruct
	//--------------------
	FontEffectBlur();
	bool Init(float const width);

	// interface
	//-----------
	void PrepareTextLayer(float const faceMultiplier, TextLayer& outLayer) const override;

	// Data
	///////
private:
	float m_Width = 0;
};


} // namespace gui
} // namespace et

