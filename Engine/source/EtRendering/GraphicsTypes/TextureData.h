#pragma once
#include "TextureParameters.h"

#include <EtCore/Content/Asset.h>
#include <EtCore/Util/LinkerUtils.h>

#include <EtRendering/GraphicsContext/GraphicsContextApi.h>


namespace et {
	REGISTRATION_NS(render);
}


namespace et {
namespace render {


//---------------------------------
// TextureData
//
// Handle to a texture object on the GPU
//
class TextureData final
{
	// definitions
	//-------------
	REGISTRATION_FRIEND_NS(render)

public:
	static constexpr uint8 s_NumCubeFaces = 6u;

	// c-tor d-tor
	//------------
private:
	TextureData() = default;
public:
	TextureData(E_ColorFormat const storageFormat, ivec2 const res, int32 const depth = 1);
	TextureData(E_TextureType const targetType, E_ColorFormat const storageFormat, ivec2 const res, int32 const depth = 1);
	~TextureData();

	// Accessors
	//----------
	T_TextureLoc GetLocation() const { return m_Location; }
	T_TextureHandle GetHandle() const { return m_Handle; }
	ivec2 GetResolution() const { return m_Resolution; }
	int32 GetNumMipLevels() const { return m_MipLevels; }

	E_ColorFormat GetStorageFormat() const { return m_StorageFormat; }
	E_TextureType GetTargetType() const { return m_TargetType; }
	int32 GetDepth() const { return m_Depth; }

	// Functionality
	//--------------
	void UploadData(void const* const data, E_ColorFormat const layout, E_DataType const dataType); // upload an image
	void UploadCompressed(void const* const data, size_t const size);
	void AllocateStorage(); // create storage on the GPU with the storage format - for framebuffers
	void SetParameters(TextureParameters const& params, bool const force = false);
	bool Resize(ivec2 const& newSize);
	void CreateHandle();

private:
	// Data
	///////

	// GPU data
	T_TextureLoc m_Location = 0u;
	T_TextureHandle m_Handle = 0u;

	// Storage info
	E_TextureType m_TargetType = E_TextureType::Texture2D;
	E_ColorFormat m_StorageFormat = E_ColorFormat::Invalid;
	ivec2 m_Resolution;
	int32 m_Depth = 1; // a (default) value of 1 implies a 2D texture
	uint8 m_MipLevels = 0u;

	// misc
	TextureParameters m_Parameters;
};


//---------------------------------
// TextureAsset
//
// Loadable Texture Data
//
class TextureAsset final : public core::Asset<TextureData, false>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(core::Asset<TextureData, false>)

public:
	// Construct destruct
	//---------------------
	TextureAsset() : core::Asset<TextureData, false>() {}
	virtual ~TextureAsset() = default;

	// Asset overrides
	//---------------------
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// Data
	///////
public:
	bool m_UseSrgb = false;
	bool m_ForceResolution = false;
	uint8 m_RequiredChannels = 0u;
	TextureParameters m_Parameters;
};


} // namespace render
} // namespace et
