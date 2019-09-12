#pragma once
#include "TextureParameters.h"

#include <glad/glad.h>

#include <EtCore/Content/Asset.h>
#include <EtCore/Helper/LinkerUtils.h>


//---------------------------------
// E_TextureType
//
// Denotes the type a texture can be
//
enum class E_TextureType : uint8
{
	Texture2D,
	Texture3D,
	CubeMap
};


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
	TextureData(int32 const width, int32 const height, int32 const internalFormat, uint32 const format, uint32 const type, int32 const depth = 1);
	TextureData(E_TextureType const targetType, int32 const height, int32 const width);
	~TextureData();

	// Accessors
	//----------
	uint32 GetHandle() const { return m_Handle; }
	ivec2 GetResolution() const { return ivec2(m_Width, m_Height); }
	int32 GetNumMipLevels() const { return m_MipLevels; }
	uint32 GetTarget() const;

	// Functionality
	//--------------
	void Build(void* data = nullptr);
	void SetParameters(TextureParameters const& params, bool const force = false);
	bool Resize(ivec2 const& newSize);

private:
	// Data
	///////

	// GPU data
	uint32 m_Handle;

	// Resolution
	int32 m_Width;
	int32 m_Height;
	int32 m_Depth = 1; // a (default) value of 1 implies a 2D texture
	uint8 m_MipLevels = 0u;
	E_TextureType m_TargetType = E_TextureType::Texture2D;

	// Format
	int32 m_InternalFormat = GL_RGB;
	uint32 m_Format = GL_RGB;
	uint32 m_Type = GL_FLOAT;

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
