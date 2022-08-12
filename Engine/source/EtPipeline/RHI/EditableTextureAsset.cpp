#include <EtRHI/stdafx.h>
#include "EditableTextureAsset.h"

#include <stb/stb_image.h>


namespace et {
namespace pl {


//========================
// Editable Texture Asset
//========================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableTextureAsset, "editable texture asset")
		.property("sRGB setting", &EditableTextureAsset::m_Srgb)
		.property("supports alpha", &EditableTextureAsset::m_SupportAlpha)
		.property("max size", &EditableTextureAsset::m_MaxSize)
		.property("use mipmaps", &EditableTextureAsset::m_UseMipMaps)
		.property("compression setting", &EditableTextureAsset::m_CompressionSetting)
		.property("compression quality", &EditableTextureAsset::m_CompressionQuality)
	END_REGISTER_CLASS_POLYMORPHIC(EditableTextureAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableTextureAsset) // force the asset class to be linked as it is only used in reflection


//---------------------------------
// EditableTextureAsset::LoadImage
//
bool EditableTextureAsset::LoadImage(RasterImage& image, std::vector<uint8> const& data)
{
	// check image format
	stbi_set_flip_vertically_on_load(false);
	int32 width = 0;
	int32 height = 0;
	int32 fileChannels = 0;

	// option to load 16 bit texture
	uint8* const pixels = stbi_load_from_memory(data.data(),
		static_cast<int32>(data.size()),
		&width,
		&height,
		&fileChannels,
		static_cast<int32>(RasterImage::s_NumChannels));
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


//--------------------------------------
// EditableTextureAsset::LoadFromMemory
//
bool EditableTextureAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	rhi::TextureAsset const* const textureAsset = static_cast<rhi::TextureAsset const*>(m_Asset.Get());

	// settings
	rhi::E_ColorFormat outputFormat = TextureCompression::GetOutputFormat(
		m_CompressionSetting, 
		m_SupportAlpha, 
		m_Srgb != rhi::TextureFormat::E_Srgb::None);
	bool const requiresCompression = rhi::TextureFormat::IsCompressedFormat(outputFormat);

	// Load
	RasterImage image;
	if (!LoadImage(image, data))
	{
		ET_WARNING("Failed to load image from data");
		return false;
	}

	uint32 width = image.GetWidth();
	uint32 height = image.GetHeight();

	// resize the texture to an appropriate format
	if (!(textureAsset->m_ForceResolution))
	{
		uint32 const size = TextureCompression::GetPow2Size(width, height, m_MaxSize);
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

	rhi::TextureData* texture = nullptr;
	if (requiresCompression)
	{
		if (m_UseMipMaps)
		{
			image.GenerateMipChain(4u); // can't compress a mipmap smaller than the block size
		}

		texture = new rhi::TextureData(outputFormat, ivec2(static_cast<int32>(width), static_cast<int32>(height)));
		RasterImage const* mipImage = &image;
		int32 mipLevel = 0;

		while (mipImage != nullptr)
		{
			std::vector<uint8> compressedData;
			if (!TextureCompression::CompressImageU8(*mipImage, outputFormat, m_CompressionQuality, compressedData))
			{
				ET_WARNING("Failed to compress image");
				return false;
			}

			texture->UploadCompressed(reinterpret_cast<void const*>(compressedData.data()), compressedData.size(), mipLevel);

			++mipLevel;
			mipImage = mipImage->GetMipChild();
		}
	}
	else
	{
		// #note: for now we cheat a bit with the storage format, but we should probably convert correctly for a more accurate representation in editor
		rhi::E_ColorFormat const storageFormat = 
			(m_Srgb == rhi::TextureFormat::E_Srgb::OnLoad) ? rhi::E_ColorFormat::SRGBA8 : rhi::E_ColorFormat::RGBA8;

		//Upload to GPU
		texture = new rhi::TextureData(storageFormat, ivec2(static_cast<int32>(width), static_cast<int32>(height)));
		texture->UploadData(reinterpret_cast<void const*>(image.GetPixels()), rhi::E_ColorFormat::RGBA, rhi::E_DataType::UByte, 0u);
	}

	texture->SetParameters(textureAsset->m_Parameters);

	// done
	SetData(texture);
	return true;
}

//----------------------------------------
// EditableTextureAsset::GenerateInternal
//
bool EditableTextureAsset::GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath)
{
	ET_UNUSED(buildConfig);
	ET_UNUSED(dbPath);

	ET_ASSERT(m_RuntimeAssets.size() == 1u);
	m_RuntimeAssets[0].m_HasGeneratedData = true; 

	rhi::TextureAsset const* const textureAsset = static_cast<rhi::TextureAsset const*>(m_Asset.Get());

	// Load
	RasterImage image;
	if (!LoadImage(image, m_Asset->GetLoadData()))
	{
		ET_ERROR("Failed to load image from data");
		return false;
	}

	// Write
	if (!TextureCompression::WriteTextureFile(m_RuntimeAssets[0].m_GeneratedData,
		image,
		m_CompressionSetting,
		m_CompressionQuality,
		m_SupportAlpha,
		m_Srgb,
		m_MaxSize,
		textureAsset->m_ForceResolution,
		m_UseMipMaps))
	{
		ET_ERROR("Failed to write texture to file");
		return false;
	}

	return true;
}


} // namespace pl
} // namespace et
