#pragma once
#include <EtRendering/GraphicsTypes/SpriteFont.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableFontAsset
//
class EditableFontAsset final : public EditorAsset<render::SpriteFont>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::SpriteFont>)
public:
	// Construct destruct
	//---------------------
	EditableFontAsset() : EditorAsset<render::SpriteFont>() {}
	virtual ~EditableFontAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	// utility
	//---------
private:
	render::SpriteFont* LoadTtf(std::vector<uint8> const& binaryContent);
};


} // namespace pl
} // namespace et
