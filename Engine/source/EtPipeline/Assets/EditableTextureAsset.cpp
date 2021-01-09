#include "stdafx.h"
#include "EditableTextureAsset.h"

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace pl {


//========================
// Editable Texture Asset
//========================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableTextureAsset, "editable texture asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableTextureAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableTextureAsset) // force the asset class to be linked as it is only used in reflection


//--------------------------------------
// EditableTextureAsset::LoadFromMemory
//
bool EditableTextureAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	render::TextureAsset const* const textureAsset = static_cast<render::TextureAsset const*>(m_Asset);

	// check image format
	stbi_set_flip_vertically_on_load(false);
	int32 width = 0;
	int32 height = 0;
	int32 fileChannels = 0;

	int32 channels = static_cast<int32>(textureAsset->m_RequiredChannels);

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

	if (!(textureAsset->m_ForceResolution))
	{
		render::GraphicsSettings const& graphicsSettings = render::RenderingSystems::Instance()->GetGraphicsSettings();
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

	bool const useSrgb = textureAsset->m_UseSrgb;

	// convert data type
	// check number of channels
	render::E_ColorFormat layout;
	render::E_ColorFormat storageFormat;
	switch (channels)
	{
	case 1:
		layout = render::E_ColorFormat::Red;
		ET_ASSERT(!useSrgb);
		storageFormat = render::E_ColorFormat::R8;
		break;

	case 2:
		layout = render::E_ColorFormat::RG;
		ET_ASSERT(!useSrgb);
		storageFormat = render::E_ColorFormat::RG8;
		break;

	case 3:
		layout = render::E_ColorFormat::RGB;
		if (useSrgb)
		{
			storageFormat = render::E_ColorFormat::SRGB8;
		}
		else
		{
			storageFormat = render::E_ColorFormat::RGB8;
		}
		break;

	case 4:
		layout = render::E_ColorFormat::RGBA;
		if (useSrgb)
		{
			storageFormat = render::E_ColorFormat::SRGBA8;
		}
		else
		{
			storageFormat = render::E_ColorFormat::RGBA8;
		}
		break;

	default:
		ET_ASSERT(false, "unhandled texture channel count");
		stbi_image_free(bits);
		return false;
	}

	//Upload to GPU
	render::TextureData* const texture = new render::TextureData(storageFormat, ivec2(width, height));
	texture->UploadData(reinterpret_cast<void const*>(bits), layout, render::E_DataType::UByte);

	stbi_image_free(bits);
	bits = nullptr;

	texture->SetParameters(textureAsset->m_Parameters);
	texture->CreateHandle();

	SetData(texture);
	return true;
}


} // namespace pl
} // namespace et
