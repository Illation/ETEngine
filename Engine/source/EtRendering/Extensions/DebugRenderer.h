#pragma once
#include <EtCore/Util/DebugUtilFwd.h>

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)

#include <EtCore/Content/AssetPointer.h>


namespace et {
namespace render {


class Camera;


//----------------
// DebugRenderer
//
// Draw lines in 3D for non shipping builds
//
class DebugRenderer final
{
	// definitions
	//-------------
private:
	friend class ShadedSceneRenderer;

	struct LineVertex
	{
		LineVertex(vec3 p, vec4 c) :pos(p), col(c) {}
		vec3 pos;
		vec4 col;
	};
	struct LineMetaData
	{
		float thickness = 1;
		uint32 start = 0;
		uint32 size = 0;
	};

	// contruct destruct
	//-------------------
	DebugRenderer() = default;
	~DebugRenderer();

	void Initialize();

	// functionality
	//---------------
public:
	void DrawLine(vec3 start, vec3 end, vec4 col = vec4(1), float thickness = 1);
	void DrawLine(vec3 start, vec4 startCol, vec3 end, vec4 endCol, float thickness = 1);

	void DrawGrid(Camera const& camera, float pixelSpacingRad = math::radians(75));

private:
	void Draw(Camera const& camera); // actually render to GPU

	// utility
	//---------
	void UpdateBuffer();
	void CheckMetaData(float thickness);


	// Data
	///////

	//Linebuffer
	std::vector<LineVertex> m_Lines;
	uint32 m_BufferSize = 0;
	rhi::T_ArrayLoc m_VAO = 0;
	rhi::T_BufferLoc m_VBO = 0;

	//Metadata
	std::vector<LineMetaData> m_MetaData;

	//Shader and its uniforms
	AssetPtr<rhi::ShaderData> m_Shader;
};


} // namespace render
} // namespace et

#endif // ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
