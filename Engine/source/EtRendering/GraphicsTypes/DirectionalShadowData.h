#pragma once

namespace et {
namespace render {


// forward
class rhi::TextureData;


//---------------------------------
// DirectionalShadowData
//
// Data to render a lights shadow with
//
class DirectionalShadowData final
{
	// definitions
	//-------------
public:

	//------------------------------------
	// DirectionalShadowData::CascadeData
	//
	// Data required for a single shadow cascade
	//
	struct CascadeData
	{
		float distance;
		mat4 lightVP;

		rhi::T_FbLoc fbo;
		rhi::TextureData* texture;
	};

	// construct destruct
	//--------------------
	DirectionalShadowData() = default;
	~DirectionalShadowData() = default;

	void Init(ivec2 const resolution);
	void Destroy();

	// accessors
	//-----------
	std::vector<CascadeData>& AccessCascades() { return m_Cascades; }
	std::vector<CascadeData> const& GetCascades() const { return m_Cascades; }

	float GetBias() const { return m_Bias; }

	// Data
	///////

private:
	std::vector<CascadeData> m_Cascades;
	float m_Bias = 0.f;
};


} // namespace render
} // namespace et
