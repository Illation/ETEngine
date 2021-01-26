#pragma once
#include <EtRendering/GraphicsTypes/EnvironmentMap.h>

#include <EtPipeline/Content/EditorAsset.h>
#include <EtPipeline/Import/TextureCompression.h>


namespace et {
	REGISTRATION_NS(pl);
}


namespace et {
namespace pl {
	

//---------------------------------
// EditableEnvironmentMapAsset
//
class EditableEnvironmentMapAsset final : public EditorAsset<render::EnvironmentMap>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::EnvironmentMap>)
	REGISTRATION_FRIEND_NS(pl)
public:
	// Construct destruct
	//---------------------
	EditableEnvironmentMapAsset() : EditorAsset<render::EnvironmentMap>() {}
	virtual ~EditableEnvironmentMapAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

private:
	void CompressHDRCube(render::TextureData*& cubeMap) const;

	// Data
	///////

	bool m_IsCompressed = false; // use BC6H
	TextureCompression::E_Quality m_CompressionQuality = TextureCompression::E_Quality::Medium;
};


} // namespace pl
} // namespace et
