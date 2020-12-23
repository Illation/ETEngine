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
};


} // namespace pl
} // namespace et
