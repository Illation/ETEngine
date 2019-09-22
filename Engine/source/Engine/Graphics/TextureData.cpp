#include "stdafx.h"
#include "TextureData.h"

#include <glad/glad.h>

#include <stb/stb_image.h>
#include <stb/stb_image_resize.h>


//==============
// Texture Data
//==============


//---------------------------------
// TextureData::c-tor
//
// Create the texture and generate a new GPU texture resource
//
TextureData::TextureData(int32 const width, int32 const height, int32 const internalFormat, uint32 const format, uint32 const type, int32 const depth)
	: m_Width(width)
	, m_Height(height)
	, m_InternalFormat(internalFormat)
	, m_Format(format)
	, m_Type(type)
	, m_Depth(depth)
	, m_TargetType((depth > 1) ? E_TextureType::Texture3D : E_TextureType::Texture2D)
{
	m_Handle = Viewport::GetCurrentApiContext()->GenerateTexture();
}

//---------------------------------
// TextureData::TextureData
//
// Create a texture of a specific type with a preexisting handle
//
TextureData::TextureData(E_TextureType const targetType, int32 const height, int32 const width)
	: m_TargetType(targetType)
	, m_Height(height)
	, m_Width(width)
	, m_InternalFormat(GL_RGB)
	, m_Format(GL_RGB)
	, m_Type(GL_FLOAT)
{
	m_Handle = Viewport::GetCurrentApiContext()->GenerateTexture();
}

//---------------------------------
// TextureData::d-tor
//
// Destroys the GPU resource
//
TextureData::~TextureData()
{
	Viewport::GetCurrentApiContext()->DeleteTexture(m_Handle);
}

//---------------------------------
// TextureData::Build
//
// send the data to the GPU location. Can be initialized without any image data
//
void TextureData::Build(void* data)
{
	Viewport::GetCurrentApiContext()->SetTextureData(*this, data);
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
	Viewport::GetCurrentApiContext()->SetTextureParams(*this, m_MipLevels, m_Parameters, params, force);
}

//---------------------------------
// TextureData::Resize
//
// returns true if regenerated 
//  - if the texture is a framebuffer texture upscaling won't work properly unless it is reatached to the framebuffer object
//
bool TextureData::Resize(ivec2 const& newSize)
{
	m_Width = newSize.x; 
	m_Height = newSize.y;

	bool const regenerate = (newSize.x > m_Width) || (newSize.y > m_Height);
	if (regenerate)
	{
		GraphicsApiContext* const api = Viewport::GetCurrentApiContext();

		api->DeleteTexture(m_Handle);
		m_Handle = api->GenerateTexture();
	}

	Build();

	if (regenerate)
	{
		SetParameters(m_Parameters, true);
	}

	return regenerate;
}


//===================
// Texture Asset
//===================


// reflection
RTTR_REGISTRATION
{
	using namespace rttr;

	registration::class_<TextureAsset>("texture asset")
		.constructor<TextureAsset const&>()
		.constructor<>()(rttr::detail::as_object())
		.property("use SRGB", &TextureAsset::m_UseSrgb)
		.property("force resolution", &TextureAsset::m_ForceResolution)
		.property("parameters", &TextureAsset::m_Parameters);

	rttr::type::register_converter_func([](TextureAsset& asset, bool& ok) -> I_Asset*
	{
		ok = true;
		return new TextureAsset(asset);
	});
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
	int32 channels = 0;

	static int32 const s_TargetNumChannels = 3; // for now we only support opaque textures

	// option to load 16 bit texture
	uint8* bits = stbi_load_from_memory(data.data(), static_cast<int32>(data.size()), &width, &height, &channels, s_TargetNumChannels);

	if (bits == nullptr)
	{
		LOG("TextureAsset::LoadFromMemory > Failed to load texture bytes from data!", LogLevel::Warning);
		return false;
	}

	if ((width == 0) || (height == 0))
	{
		LOG("TextureAsset::LoadFromMemory > Image is too small to display!", LogLevel::Warning);
		stbi_image_free(bits);
		return false;
	}

	Config::Settings::Graphics const& graphicsSettings = Config::GetInstance()->GetGraphics();
	if (!etm::nearEquals(graphicsSettings.TextureScaleFactor, 1.f))
	{
		if (!m_ForceResolution)
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

	//Upload to GPU
	m_Data = new TextureData(width, height, m_UseSrgb ? GL_SRGB : GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
	m_Data->Build((void*)bits);
	m_Data->SetParameters(m_Parameters);

	stbi_image_free(bits);
	bits = nullptr;

	return true;
}
