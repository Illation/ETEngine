#pragma once
#include <EtRendering/GraphicsTypes/EnvironmentMap.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableEnvironmentMapAsset
//
class EditableEnvironmentMapAsset final : public EditorAsset<render::EnvironmentMap>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::EnvironmentMap>)
public:
	// Construct destruct
	//---------------------
	EditableEnvironmentMapAsset() : EditorAsset<render::EnvironmentMap>() {}
	virtual ~EditableEnvironmentMapAsset() = default;
};


} // namespace pl
} // namespace et
