#pragma once
#include <EtRendering/GraphicsTypes/TextureData.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableTextureAsset
//
class EditableTextureAsset final : public EditorAsset<render::TextureData>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::TextureData>)
public:
	// Construct destruct
	//---------------------
	EditableTextureAsset() : EditorAsset<render::TextureData>() {}
	virtual ~EditableTextureAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;
};


} // namespace pl
} // namespace et
