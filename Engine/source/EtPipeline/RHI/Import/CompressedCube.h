#pragma once
#include <EtRHI/GraphicsTypes/TextureData.h>

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
	friend class Create<CompressedCube>;

	// construct destruct
	//--------------------
	CompressedCube() = default;
public:
	CompressedCube(rhi::TextureData const& cubeMap, TextureCompression::E_Quality const quality);
	~CompressedCube() = default;
protected:

	// functionality
	//---------------
	void CompressFromTexture(rhi::TextureData const& cubeMap, 
		TextureCompression::E_Quality const quality, 
		uint8 const mipLevel, 
		uint32 const size);
	void CreateMip();

	// accessors
	//-----------
	CompressedCube* GetChildMip() { return m_ChildMip.Get(); }
public:
	CompressedCube const* GetChildMip() const { return m_ChildMip.Get(); }
	std::vector<uint8> const& GetCompressedData() const { return m_CompressedData; }

	// Data
	///////

private:
	std::vector<uint8> m_CompressedData;
	UniquePtr<CompressedCube> m_ChildMip;
};


} // namespace pl
} // namespace et

