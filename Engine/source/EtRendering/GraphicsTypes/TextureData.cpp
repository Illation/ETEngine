#include "stdafx.h"
#include "TextureData.h"

#include <EtBuild/EngineVersion.h>

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Reflection/Registration.h>
#include <EtCore/IO/BinaryReader.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>

#include "TextureFormat.h"


namespace et {
namespace render {


//==============
// Texture Data
//==============


//---------------------------------
// TextureData::c-tor
//
// Create the texture and generate a new GPU texture resource
//
TextureData::TextureData(E_ColorFormat const storageFormat, ivec2 const res, int32 const depth)
	: m_TargetType((depth > 1) ? E_TextureType::Texture3D : E_TextureType::Texture2D)
	, m_StorageFormat(storageFormat)
	, m_Resolution(res)
	, m_Depth(depth)
{
	m_Location = ContextHolder::GetRenderContext()->GenerateTexture();
}

//---------------------------------
// TextureData::TextureData
//
// Create a texture of a specific type with a preexisting handle
//
TextureData::TextureData(E_TextureType const targetType, E_ColorFormat const storageFormat, ivec2 const res, int32 const depth)
	: m_TargetType(targetType)
	, m_StorageFormat(storageFormat)
	, m_Resolution(res)
	, m_Depth(depth)
{
	m_Location = ContextHolder::GetRenderContext()->GenerateTexture();
}

//---------------------------------
// TextureData::d-tor
//
// Destroys the GPU resource
//
TextureData::~TextureData()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();
	if (m_Handle != 0u)
	{
		api->SetTextureHandleResidency(m_Handle, false);
	}

	api->DeleteTexture(m_Location);
}

//---------------------------------
// TextureData::UploadData
//
// Send an image bitmap to the GPU location
//
void TextureData::UploadData(void const* const data, E_ColorFormat const layout, E_DataType const dataType, int32 const mipLevel)
{
	m_MipLevels = std::max(static_cast<uint8>(mipLevel), m_MipLevels);

	ET_ASSERT(m_Handle == 0u, "Shouldn't upload data after a handle was created!");
	ContextHolder::GetRenderContext()->UploadTextureData(*this, data, layout, dataType, mipLevel);
}

//---------------------------------
// TextureData::UploadCompressed
//
// Send a compressed image to the GPU location, matching the codec specified for the internal storage format
//
void TextureData::UploadCompressed(void const* const data, size_t const size, int32 const mipLevel)
{
	m_MipLevels = std::max(static_cast<uint8>(mipLevel), m_MipLevels);

	ET_ASSERT(m_Handle == 0u, "Shouldn't upload data after a handle was created!");
	ContextHolder::GetRenderContext()->UploadCompressedTextureData(*this, data, size, mipLevel);
}

//---------------------------------
// TextureData::AllocateStorage
//
// Allocate empty texture storage at the GPU location. The actual texture data is assumed to be generated
//
void TextureData::AllocateStorage()
{
	ET_ASSERT(m_Handle == 0u, "Shouldn't upload data after a handle was created!");
	ContextHolder::GetRenderContext()->AllocateTextureStorage(*this);
}

//---------------------------------
// TextureData::SetParameters
//
// Sets parameters on a texture, can optionally be enforced
// This function should be called either way when a new texture is created, 
//  - because if we leave the default parameters OpenGL expects us to generate a mip map, or we can disable mip maps which will change the min filter
//
void TextureData::SetParameters(TextureParameters const& params, bool const force)
{
	ET_ASSERT(m_Handle == 0u, "Shouldn't set parameters after a handle was created!");

	ContextHolder::GetRenderContext()->SetTextureParams(*this, m_Parameters, params, force);
}

//---------------------------------
// TextureData::GenerateMipMaps
//
// (on GPU)
//
void TextureData::GenerateMipMaps()
{
	ET_ASSERT(m_Handle == 0u, "Shouldn't generate mip maps after a handle was created!");

	ContextHolder::GetRenderContext()->GenerateMipMaps(*this, m_MipLevels);
}

//---------------------------------
// TextureData::Resize
//
// returns true if regenerated 
//  - if the texture is a framebuffer texture upscaling won't work properly unless it is reatached to the framebuffer object
//
bool TextureData::Resize(ivec2 const& newSize)
{
	bool const hasHandle = (m_Handle != 0u);

	bool const regenerate = (newSize.x > m_Resolution.x) || (newSize.y > m_Resolution.y) || hasHandle;

	m_Resolution = newSize;

	if (regenerate)
	{
		I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

		if (hasHandle)
		{
			api->SetTextureHandleResidency(m_Handle, false);
			m_Handle = 0u;
		}

		api->DeleteTexture(m_Location);
		m_Location = api->GenerateTexture();
	}

	AllocateStorage();

	if (regenerate)
	{
		SetParameters(m_Parameters, true);
	}

	if (hasHandle)
	{
		CreateHandle();
	}

	return regenerate;
}

//---------------------------------
// TextureData::CreateHandle
//
// Create a handle that allows bindless use of the texture. After the handle is created, no other modifying operations should be done (except resize)
//
void TextureData::CreateHandle()
{
	I_GraphicsContextApi* const api = ContextHolder::GetRenderContext();

	m_Handle = api->GetTextureHandle(m_Location);
	api->SetTextureHandleResidency(m_Handle, true); // #todo: in the future we should have a system that makes inactive handles non resident after a while
}


//===================
// Texture Asset
//===================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS_ASSET(TextureData, "texture data")
	END_REGISTER_CLASS(TextureData);

	BEGIN_REGISTER_CLASS(TextureAsset, "texture asset")
		.property("force resolution", &TextureAsset::m_ForceResolution)
		.property("parameters", &TextureAsset::m_Parameters)
	END_REGISTER_CLASS_POLYMORPHIC(TextureAsset, core::I_Asset);
}
DEFINE_FORCED_LINKING(TextureAsset) // force the shader class to be linked as it is only used in reflection


//---------------------------------
// TextureAsset::LoadFromMemory
//
// Load texture data from binary asset content, and place it on the GPU
//
bool TextureAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	core::BinaryReader reader;
	reader.Open(data);
	ET_ASSERT(reader.Exists());

	// read header
	//-------------
	if (reader.ReadString(TextureFormat::s_Header.size()) != TextureFormat::s_Header)
	{
		ET_ASSERT(false, "Incorrect texture file header");
		return false;
	}

	std::string const writerVersion = reader.ReadNullString();
	if (writerVersion != build::Version::s_Name)
	{
		LOG(FS("Texture data was written by a different engine version: %s", writerVersion.c_str()));
	}

	// read texture info
	//-------------------
	E_TextureType const targetType = reader.Read<E_TextureType>();
	if (!((targetType == E_TextureType::Texture2D) || (targetType == E_TextureType::CubeMap)))
	{
		ET_ASSERT(false, "Only 2D texture assets and cubemaps are currently supported!");
		return false;
	}

	uint16 const width = reader.Read<uint16>();
	uint16 const height = reader.Read<uint16>();

	uint16 const layers = reader.Read<uint16>();
	ET_ASSERT(((targetType != E_TextureType::Texture3D) && (layers == 1u)) || (layers > 0u));

	uint8 const mipCount = reader.Read<uint8>();

	E_ColorFormat const storageFormat = reader.Read<E_ColorFormat>();
	bool const isCompressed = TextureFormat::IsCompressedFormat(storageFormat);

	E_DataType const dataType = reader.Read<E_DataType>();
	ET_ASSERT(!isCompressed || dataType == E_DataType::Invalid);
	E_ColorFormat const layout = reader.Read<E_ColorFormat>();
	ET_ASSERT(!isCompressed || layout == E_ColorFormat::Invalid);

	// #todo: respect GraphicsSetting texture resizing by only loading lower mip levels

	// Upload to GPU
	//---------------
	m_Data = new TextureData(targetType, storageFormat, ivec2(static_cast<int32>(width), static_cast<int32>(height)), static_cast<int32>(layers));

	uint8 const* dataPointer = reader.GetCurrentDataPointer();
	if (isCompressed)
	{
		size_t mipSize = TextureFormat::GetCompressedSize(static_cast<uint32>(width), static_cast<uint32>(height), storageFormat);
		if (targetType == E_TextureType::CubeMap)
		{
			mipSize *= TextureData::s_NumCubeFaces;
		}

		int32 mipLevel = 0;
		for (;;)
		{
			m_Data->UploadCompressed(reinterpret_cast<void const*>(dataPointer), mipSize, mipLevel);

			if (mipLevel == static_cast<int32>(mipCount))
			{
				break;
			}

			reader.MoveBufferPosition(mipSize);
			mipSize /= 4u;

			dataPointer = reader.GetCurrentDataPointer();
			++mipLevel;
		}
	}
	else
	{
		size_t const pixelSize = static_cast<size_t>(TextureFormat::GetChannelCount(layout)) * static_cast<size_t>(DataTypeInfo::GetTypeSize(dataType));
		size_t mipSize = pixelSize * static_cast<size_t>(width) * static_cast<size_t>(height);
		if (targetType == E_TextureType::CubeMap)
		{
			mipSize *= TextureData::s_NumCubeFaces;
		}

		int32 mipLevel = 0;
		for (;;)
		{
			m_Data->UploadData(reinterpret_cast<void const*>(dataPointer), layout, dataType, mipLevel);

			if (mipLevel == static_cast<int32>(mipCount))
			{
				break;
			}

			reader.MoveBufferPosition(mipSize);
			mipSize /= 4u;

			dataPointer = reader.GetCurrentDataPointer();
			++mipLevel;
		}
	}

	m_Data->SetParameters(m_Parameters);
	if (targetType != E_TextureType::CubeMap)
	{
		m_Data->CreateHandle();
	}

	return true;
}


} // namespace render
} // namespace et
