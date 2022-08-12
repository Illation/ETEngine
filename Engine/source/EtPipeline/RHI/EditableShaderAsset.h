#pragma once
#include <EtRHI/GraphicsTypes/Shader.h>

#include <EtPipeline/Core/Content/EditorAsset.h>


namespace et {
	REGISTRATION_NS(pl);
}


namespace et {
namespace pl {
	

//---------------------------------
// EditableShaderAsset
//
class EditableShaderAsset final : public EditorAsset<rhi::ShaderData>
{
	RTTR_ENABLE(EditorAsset<rhi::ShaderData>)
	REGISTRATION_FRIEND_NS(pl)
	DECLARE_FORCED_LINKING()
public:
	// Construct destruct
	//---------------------
	EditableShaderAsset() : EditorAsset<rhi::ShaderData>() {}
	virtual ~EditableShaderAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	void SetupRuntimeAssetsInternal() override;
	bool GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath) override;

	bool GenerateRequiresLoadData() const override { return true; }
	bool GenerateRequiresReferences() const override { return true; }

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
