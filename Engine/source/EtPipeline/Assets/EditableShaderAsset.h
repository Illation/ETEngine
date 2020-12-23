#pragma once
#include <EtRendering/GraphicsTypes/Shader.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
namespace pl {
	

//---------------------------------
// EditableShaderAsset
//
class EditableShaderAsset final : public EditorAsset<render::ShaderData>
{
	DECLARE_FORCED_LINKING()
	RTTR_ENABLE(EditorAsset<render::ShaderData>)
public:
	// Construct destruct
	//---------------------
	EditableShaderAsset() : EditorAsset<render::ShaderData>() {}
	virtual ~EditableShaderAsset() = default;
};


} // namespace pl
} // namespace et
