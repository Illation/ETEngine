#pragma once
#include <EtRendering/GraphicsTypes/TextureData.h>

#include "TextureCompression.h"


namespace et {
namespace pl {


//--------------------------
// CompressedCube
//
// Container for compressed HDR cube map data
//
class CompressedCube final
{
	// construct destruct
	//--------------------
	CompressedCube() = default;
public:
	CompressedCube(render::TextureData const& cubeMap, TextureCompression::E_Quality const quality);
	~CompressedCube();
protected:

	// functionality
	//---------------
	void CompressFromTexture(render::TextureData const& cubeMap, 
		TextureCompression::E_Quality const quality, 
		uint8 const mipLevel, 
		uint32 const size);
	void CreateMip();

	// accessors
	//-----------
	CompressedCube* GetChildMip() { return m_ChildMip; }
public:
	CompressedCube const* GetChildMip() const { return m_ChildMip; }
	std::vector<uint8> const& GetCompressedData() const { return m_CompressedData; }

	// Data
	///////

private:
	std::vector<uint8> m_CompressedData;
	CompressedCube* m_ChildMip = nullptr;
};


} // namespace pl
} // namespace et

