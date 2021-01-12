#include "stdafx.h"
#include "EditableTextureAsset.h"

#include <stb/stb_image.h>

#include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>


namespace et {
namespace pl {


//========================
// Editable Texture Asset
//========================


// reflection
RTTR_REGISTRATION
{
	rttr::registration::enumeration<EditableTextureAsset::E_CompressionQuality>("E_CompressionQuality") (
		rttr::value("Low", EditableTextureAsset::E_CompressionQuality::Low),
		rttr::value("Medium", EditableTextureAsset::E_CompressionQuality::Medium),
		rttr::value("High", EditableTextureAsset::E_CompressionQuality::High),
		rttr::value("Ultra", EditableTextureAsset::E_CompressionQuality::Ultra));

	BEGIN_REGISTER_CLASS(EditableTextureAsset, "editable texture asset")
		.property("sRGB setting", &EditableTextureAsset::m_Srgb)
		.property("supports alpha", &EditableTextureAsset::m_SupportAlpha)
		.property("max size", &EditableTextureAsset::m_MaxSize)
		.property("compression setting", &EditableTextureAsset::m_CompressionSetting)
		.property("compression quality", &EditableTextureAsset::m_CompressionQuality)
	END_REGISTER_CLASS_POLYMORPHIC(EditableTextureAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableTextureAsset) // force the asset class to be linked as it is only used in reflection


//--------------------------------------
// EditableTextureAsset::LoadFromMemory
//
bool EditableTextureAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	render::TextureAsset const* const textureAsset = static_cast<render::TextureAsset const*>(m_Asset);

	// settings
	ET_ASSERT((m_Srgb != render::E_SrgbSetting::None) == textureAsset->m_UseSrgb);
	render::E_ColorFormat outputFormat = GetOutputFormat(m_CompressionSetting, m_SupportAlpha, m_Srgb != render::E_SrgbSetting::None);
	bool const requiresCompression = render::RequiresCompression(m_CompressionSetting);

	// Load
	RasterImage image;
	if (!LoadImage(image, data))
	{
		ET_ASSERT(false, "Failed to load image from data");
		return false;
	}

	uint32 width = image.GetWidth();
	uint32 height = image.GetHeight();

	// resize the texture to an appropriate format
	if (!(textureAsset->m_ForceResolution))
	{
		uint32 const size = GetPow2Size(width, height, true);
		if ((width != size) || (height != size))
		{
			image.Resize(size, size);
			width = image.GetWidth();
			height = image.GetHeight();
		}
	}
	else
	{
		ET_ASSERT(!requiresCompression, "texture '%s' forces resolution but requires compression", m_Id.ToStringDbg());
	}

	// generate mipmaps here in the future

	//if (requiresCompression)
	//{
	//	uint32 const blocksX = width / 4;
	//	uint32 const blocksY = height / 4;

	//	std::vector<Block8> packedImage8(blocksX * blocksY);
	//	std::vector<Block16> packedImage16(blocksX * blocksY);
	//}
	//else
	//{
	//	// swizzle(2, 1, 0, 3)
	//}

	// temp
	render::E_ColorFormat const layout = render::E_ColorFormat::RGBA;
	render::E_ColorFormat const storageFormat = (m_Srgb == render::E_SrgbSetting::OnLoad) ? render::E_ColorFormat::SRGBA8 : render::E_ColorFormat::RGBA8;

	//Upload to GPU
	render::TextureData* const texture = new render::TextureData(storageFormat, ivec2(static_cast<int32>(width), static_cast<int32>(height)));
	texture->UploadData(reinterpret_cast<void const*>(image.GetPixels()), layout, render::E_DataType::UByte);

	texture->SetParameters(textureAsset->m_Parameters);
	texture->CreateHandle();

	// done
	SetData(texture);
	return true;
}

//---------------------------------
// EditableTextureAsset::LoadImage
//
bool EditableTextureAsset::LoadImage(RasterImage& image, std::vector<uint8> const& data) const
{
	// check image format
	stbi_set_flip_vertically_on_load(false);
	int32 width = 0;
	int32 height = 0;
	int32 fileChannels = 0;

	// option to load 16 bit texture
	uint8* const pixels = stbi_load_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &fileChannels, RasterImage::s_NumChannels);
	if ((pixels == nullptr) || (width == 0) || (height == 0))
	{
		return false;
	}

	image.SetSize(static_cast<uint32>(width), static_cast<uint32>(height));
	image.AllocatePixels();
	image.SetPixels(reinterpret_cast<RasterImage::ColorU8 const*>(pixels));

	stbi_image_free(pixels);
	 
	return true;
}

//-----------------------------------
// EditableTextureAsset::GetPow2Size
//
// Return the nearest power of two of the larger edge, clamped to the max size and adjusted by graphics settings
//
uint32 EditableTextureAsset::GetPow2Size(uint32 const width, uint32 const height, bool adjustByGraphicsSettings) const
{
	uint32 size = std::max(width, height);
	if (m_MaxSize != 0u)
	{
		size = std::min(static_cast<uint32>(m_MaxSize), size);
	}

	if (adjustByGraphicsSettings)
	{
		render::GraphicsSettings const& graphicsSettings = render::RenderingSystems::Instance()->GetGraphicsSettings();
		if (!math::nearEquals(graphicsSettings.TextureScaleFactor, 1.f))
		{
			size = static_cast<uint32>(static_cast<float>(size) * graphicsSettings.TextureScaleFactor);
		}
	}

	return RasterImage::GetClosestPowerOf2(size);
}

//---------------------------------------
// EditableTextureAsset::GetOutputFormat
//
render::E_ColorFormat EditableTextureAsset::GetOutputFormat(render::E_CompressionSetting const setting, bool const supportAlpha, bool const useSrgb) const
{
	switch (setting)
	{
	case render::E_CompressionSetting::Default:
		if (useSrgb)
		{
			if (supportAlpha)
			{
				return render::E_ColorFormat::BC3_SRGBA;
			}

			return render::E_ColorFormat::BC1_SRGB; 
		}

		if (supportAlpha)
		{
			return render::E_ColorFormat::BC3_RGBA;
		}

		return render::E_ColorFormat::BC1_RGB; // in the future we might allow for one bit alpha

	case render::E_CompressionSetting::NormalMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::BC5_RG;

	case render::E_CompressionSetting::GrayScale:
		ET_ASSERT(!supportAlpha);
		if (useSrgb)
		{
			return render::E_ColorFormat::SRGB8;
		}

		return render::E_ColorFormat::R8;

	case render::E_CompressionSetting::DisplacementMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::R8; // in the future maybe also support R16, but there is no point now because we load 8 bits

	case render::E_CompressionSetting::VectorDisplacementMap:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::RGB8;

	case render::E_CompressionSetting::HDR:
		ET_ASSERT(!supportAlpha);
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::RGB16f;

	case render::E_CompressionSetting::UI:
		ET_ASSERT(supportAlpha);
		if (useSrgb)
		{
			return render::E_ColorFormat::SRGBA8;
		}

		return render::E_ColorFormat::RGBA8;

	case render::E_CompressionSetting::Alpha:
		ET_ASSERT(!useSrgb);
		return render::E_ColorFormat::BC4_Red; // signed?

	case render::E_CompressionSetting::SdfFont:
		ET_ASSERT(!useSrgb);
		ET_ASSERT(supportAlpha);
		return render::E_ColorFormat::RGBA8; 

	case render::E_CompressionSetting::BC7:
		ET_ASSERT(supportAlpha);
		if (useSrgb)
		{
			return render::E_ColorFormat::BC7_SRGBA;
		}

		return render::E_ColorFormat::BC7_RGBA;
	}

	ET_ASSERT(false, "unhandled compression setting");
	return render::E_ColorFormat::Invalid;
}

//--------------------------------------------
// EditableTextureAsset::GetInputChannelCount
//
uint8 EditableTextureAsset::GetInputChannelCount(render::E_ColorFormat const format) const
{
	switch (format)
	{
	case render::E_ColorFormat::R8:
	case render::E_ColorFormat::BC4_Red:
		return 1u;

	case render::E_ColorFormat::BC5_RG:
		return 2u;

	case render::E_ColorFormat::RGB8:
	case render::E_ColorFormat::SRGB8:
	case render::E_ColorFormat::RGB16f:
	case render::E_ColorFormat::BC1_RGB:
	case render::E_ColorFormat::BC1_SRGB:
		return 3u;

	case render::E_ColorFormat::RGBA8:
	case render::E_ColorFormat::SRGBA8:
	case render::E_ColorFormat::BC3_RGBA:
	case render::E_ColorFormat::BC3_SRGBA:
	case render::E_ColorFormat::BC7_RGBA:
	case render::E_ColorFormat::BC7_SRGBA:
		return 4u;

	default: 
		ET_ASSERT(false, "unhandled color format");
		return 0u;
	}
}


} // namespace pl
} // namespace et
