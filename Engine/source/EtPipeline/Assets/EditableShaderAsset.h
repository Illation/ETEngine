#pragma once
#include <EtRendering/GraphicsTypes/Shader.h>

#include <EtPipeline/Content/EditorAsset.h>


namespace et {
	REGISTRATION_NS(pl);
}


namespace et {
namespace pl {
	

//---------------------------------
// EditableShaderAsset
//
class EditableShaderAsset final : public EditorAsset<render::ShaderData>
{
	RTTR_ENABLE(EditorAsset<render::ShaderData>)
	REGISTRATION_FRIEND_NS(pl)
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	EditableShaderAsset() : EditorAsset<render::ShaderData>() {}
	virtual ~EditableShaderAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;
	//void SetupRuntimeAssetsInternal() override;
	//void GenerateInternal(BuildConfiguration const& buildConfig) override;

	// utility
	//---------
private:
	bool Precompile(std::string &shaderContent, std::string &vertSource, std::string &geoSource, std::string &fragSource);

	bool ReplaceInclude(std::string &line);

	// Data
	///////

	bool m_UseGeometry = false;
	bool m_UseFragment = true;
};


} // namespace pl
} // namespace et
