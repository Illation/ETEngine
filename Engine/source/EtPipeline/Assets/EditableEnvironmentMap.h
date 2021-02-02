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

	static std::string const s_EnvMapPostFix;
	static std::string const s_IrradiancePostFix;
	static std::string const s_RadiancePostFix;
public:
	// Construct destruct
	//---------------------
	EditableEnvironmentMapAsset() : EditorAsset<render::EnvironmentMap>() {}
	virtual ~EditableEnvironmentMapAsset() = default;

	// interface
	//-----------
protected:
	bool LoadFromMemory(std::vector<uint8> const& data) override;

	void SetupRuntimeAssetsInternal() override;

	bool GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath) override;
	bool GenerateRequiresLoadData() const override { return true; }

private:
	bool CreateTextures(std::vector<uint8> const& data,
		render::TextureData*& env, 
		render::TextureData*& irradiance, 
		render::TextureData*& radiance) const;
	void CompressHDRCube(render::TextureData*& cubeMap) const;

	bool GenerateTextureData(std::vector<uint8>& data, render::TextureData const* const texture) const;
	void GenerateBinEnvMap(std::vector<uint8>& data,
		core::HashString const env, 
		core::HashString const irradiance, 
		core::HashString const radiance) const;


	// Data
	///////

	bool m_IsCompressed = false; // use BC6H
	TextureCompression::E_Quality m_CompressionQuality = TextureCompression::E_Quality::Medium;
	int32 m_CubemapRes = 1024;
	int32 m_IrradianceRes = 32;
	int32 m_RadianceRes = 1024;
};


} // namespace pl
} // namespace et
