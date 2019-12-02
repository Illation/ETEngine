#pragma once
#include "TextureParameters.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Helper/LinkerUtils.h>

#include <EtRendering/GraphicsContext/GraphicsApiContext.h>


//---------------------------------
// TextureData
//
// Handle to a texture object on the GPU
//
class TextureData final
{
public:
	// definitions
	static constexpr uint8 s_NumCubeFaces = 6u;

	// c-tor d-tor
	//------------
	TextureData(ivec2 const res, E_ColorFormat const intern, E_ColorFormat const format, E_DataType const type, int32 const depth = 1);
	TextureData(E_TextureType const targetType, ivec2 const res);
	~TextureData();

	// Accessors
	//----------
	T_TextureLoc GetLocation() const { return m_Location; }
	ivec2 GetResolution() const { return m_Resolution; }
	int32 GetNumMipLevels() const { return m_MipLevels; }

	E_ColorFormat GetInternalFormat() const { return m_Internal; }
	E_ColorFormat GetFormat() const { return m_Format; }
	E_DataType GetDataType() const { return m_DataType; }

	E_TextureType GetTargetType() const { return m_TargetType; }
	int32 GetDepth() const { return m_Depth; }

	// Functionality
	//--------------
	void Build(void* data = nullptr);
	void SetParameters(TextureParameters const& params, bool const force = false);
	bool Resize(ivec2 const& newSize);
	void CreateHandle();

private:
	// Data
	///////

	// GPU data
	T_TextureLoc m_Location;
	T_TextureHandle m_Handle = 0u;

	// Resolution
	ivec2 m_Resolution;
	int32 m_Depth = 1; // a (default) value of 1 implies a 2D texture

	uint8 m_MipLevels = 0u;
	E_TextureType m_TargetType = E_TextureType::Texture2D;

	// Format - #note: these are overridden by cubemaps!
	E_ColorFormat m_Internal;
	E_ColorFormat m_Format;
	E_DataType m_DataType;

	TextureParameters m_Parameters;
};


//---------------------------------
// TextureAsset
//
// Loadable Texture Data
//
class TextureAsset final : public Asset<TextureData, false>
{
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	TextureAsset() : Asset<TextureData, false>() {}
	virtual ~TextureAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Data
	///////
public:
	bool m_UseSrgb = false;
	bool m_ForceResolution = false;
	TextureParameters m_Parameters;

	RTTR_ENABLE(Asset<TextureData, false>)
};
