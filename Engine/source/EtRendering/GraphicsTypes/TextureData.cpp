#include "stdafx.h"
#include "TextureData.h"

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <EtCore/Content/AssetRegistration.h>
#include <EtCore/Reflection/Registration.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


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
void TextureData::UploadData(void const* const data, E_ColorFormat const layout, E_DataType const dataType)
{
	ET_ASSERT(m_Handle == 0u, "Shouldn't upload data after a handle was created!");
	ContextHolder::GetRenderContext()->UploadTextureData(*this, data, layout, dataType);
}

//---------------------------------
// TextureData::UploadCompressed
//
// Send a compressed image to the GPU location, matching the codec specified for the internal storage format
//
void TextureData::UploadCompressed(void const* const data, size_t const size)
{
	ET_ASSERT(m_Handle == 0u, "Shouldn't upload data after a handle was created!");
	ContextHolder::GetRenderContext()->UploadCompressedTextureData(*this, data, size);
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

	ContextHolder::GetRenderContext()->SetTextureParams(*this, m_MipLevels, m_Parameters, params, force);
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
		.property("use SRGB", &TextureAsset::m_UseSrgb)
		.property("force resolution", &TextureAsset::m_ForceResolution)
		.property("required channels", &TextureAsset::m_RequiredChannels)
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
	// check image format

	stbi_set_flip_vertically_on_load(false);
	int32 width = 0;
	int32 height = 0;
	int32 fileChannels = 0;

	int32 channels = static_cast<int32>(m_RequiredChannels);

	// option to load 16 bit texture
	uint8* bits = stbi_load_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &fileChannels, channels);

	if (channels == 0)
	{
		channels = fileChannels;
	}

	if (bits == nullptr)
	{
		LOG("TextureAsset::LoadFromMemory > Failed to load texture bytes from data!", core::LogLevel::Warning);
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		LOG("TextureAsset::LoadFromMemory > Image is too small to display!", core::LogLevel::Warning);
		stbi_image_free(bits);
		return false;
	}

	if (!m_ForceResolution)
	{
		render::GraphicsSettings const& graphicsSettings = RenderingSystems::Instance()->GetGraphicsSettings();
		if (!math::nearEquals(graphicsSettings.TextureScaleFactor, 1.f))
		{
			// resize
			int32 const outWidth = static_cast<int32>(static_cast<float>(width) * graphicsSettings.TextureScaleFactor);
			int32 const outHeight = static_cast<int32>(static_cast<float>(height) * graphicsSettings.TextureScaleFactor);
			uint8* outBits = new uint8[outWidth * outHeight * channels];

			stbir_resize_uint8(bits, width, height, 0, outBits, outWidth, outHeight, 0, channels);

			stbi_image_free(bits);
			bits = outBits;
			width = outWidth;
			height = outHeight;
		}
	}

	// convert data type
	// check number of channels
	E_ColorFormat layout;
	E_ColorFormat storageFormat;
	switch (channels)
	{
	case 1:
		layout = E_ColorFormat::Red;
		ET_ASSERT(!m_UseSrgb);
		storageFormat = E_ColorFormat::R8;
		break;

	case 2:
		layout = E_ColorFormat::RG;
		ET_ASSERT(!m_UseSrgb);
		storageFormat = E_ColorFormat::RG8;
		break;

	case 3:
		layout = E_ColorFormat::RGB;
		if (m_UseSrgb)
		{
			storageFormat = E_ColorFormat::SRGB8;
		}
		else
		{
			storageFormat = E_ColorFormat::RGB8;
		}
		break;

	case 4:
		layout = E_ColorFormat::RGBA;
		if (m_UseSrgb)
		{
			storageFormat = E_ColorFormat::SRGBA8;
		}
		else
		{
			storageFormat = E_ColorFormat::RGBA8;
		}
		break;

	default:
		ET_ASSERT(false, "unhandled texture channel count");
		stbi_image_free(bits);
		return false;
	}

	//Upload to GPU
	m_Data = new TextureData(storageFormat, ivec2(width, height));
	m_Data->UploadData(reinterpret_cast<void const*>(bits), layout, E_DataType::UByte);
	m_Data->SetParameters(m_Parameters);

	m_Data->CreateHandle();

	stbi_image_free(bits);
	bits = nullptr;

	return true;
}


} // namespace render
} // namespace et
