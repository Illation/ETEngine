#include "stdafx.h"
#include "Triangulator.h"

#include "Planet.h"

#include <EtCore/Util/DebugUtilFwd.h>

#include <EtRendering/GraphicsTypes/Frustum.h>
#include <EtRendering/SceneRendering/ShadedSceneRenderer.h>

#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
#	include <EtRendering/GlobalRenderingSystems/GlobalRenderingSystems.h>
#endif


namespace et {
namespace render {


void Triangulator::Init(Planet* const planet)
{
	m_Planet = planet;

	auto ico = math::GetIcosahedronPositions(m_Planet->GetRadius());
	auto indices = math::GetIcosahedronIndices();
	for (size_t i = 0; i < indices.size(); i+=3)
	{
		m_Icosahedron.push_back(Tri(ico[indices[i]], ico[indices[i+1]], ico[indices[i+2]], nullptr, 0));
	}

	Precalculate();
}

bool Triangulator::Update(mat4 const& transform, Camera const& camera)
{
	m_MaxLevel = 22;
	Precalculate();

	//Frustum update
	m_Frustum.SetCullTransform(transform);
#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	if (!(RenderingSystems::Instance()->GetDebugVars().IsFrustumFrozen()))
	{
#endif
		m_Frustum.SetToCamera(camera);
#if ET_CT_IS_ENABLED(ET_CT_DBG_UTIL)
	}
#endif

	m_Frustum.Update(rhi::Viewport::GetCurrentViewport());

	return true;
}

void Triangulator::Precalculate()
{
	//determine culling angle behind planet based on max height
	float cullingAngle = acosf(m_Planet->GetRadius()/(m_Planet->GetRadius()+m_Planet->GetMaxHeight()));
	//Dot Product LUT
	m_TriLevelDotLUT.clear();
	m_TriLevelDotLUT.push_back(0.5f+sinf(cullingAngle));
	float angle = acosf(0.5f);
	for (int32 i = 1; i <= m_MaxLevel; i++)
	{
		angle *= 0.5f;
		m_TriLevelDotLUT.push_back(sinf(angle+cullingAngle));
	}
	//height multipliers
	m_HeightMultLUT.clear();
	vec3 a = m_Icosahedron[0].a;
	vec3 b = m_Icosahedron[0].b;
	vec3 c = m_Icosahedron[0].c;
	vec3 center = (a + b + c) / 3.f;
	center = center * m_Planet->GetRadius() / math::length(center);//+maxHeight
	m_HeightMultLUT.push_back(1 / math::dot( math::normalize(a), math::normalize(center)));
	float normMaxHeight = m_Planet->GetMaxHeight() / m_Planet->GetRadius();
	for (int32 i = 1; i <= m_MaxLevel; i++)
	{
		vec3 A = b + ((c - b)*0.5f);
		vec3 B = c + ((a - c)*0.5f);
		c = a + ((b - a)*0.5f);
		a = A * m_Planet->GetRadius() / math::length(A);
		b = B * m_Planet->GetRadius() / math::length(B);
		c = c * m_Planet->GetRadius() / math::length(c);
		m_HeightMultLUT.push_back(1 / math::dot( math::normalize(a), math::normalize(center)) + normMaxHeight);
	}
}

void Triangulator::GenerateGeometry()
{
	//Precalculate Distance LUT
	//The distances generated should keep the triangles smaller than m_AllowedTriPx at any level
	//In future only recalculate on FOV or triangle density change
	m_DistanceLUT.clear();
	float sizeL = math::length(m_Icosahedron[0].a - m_Icosahedron[0].b);
	float frac = tanf((m_AllowedTriPx * math::radians(m_Frustum.GetFOV())) / rhi::Viewport::GetCurrentViewport()->GetDimensions().x);
	for (int32 level = 0; level < m_MaxLevel+5; level++)
	{
		m_DistanceLUT.push_back(sizeL / frac);
		sizeL *= 0.5f;
	}

	//Recursion start
	m_Positions.clear();

	for (auto t : m_Icosahedron)
	{
		RecursiveTriangle(t.a, t.b, t.c, t.level, true);
	}
}

TriNext Triangulator::SplitHeuristic(vec3 &a, vec3 &b, vec3 &c, int16 level, bool frustumCull)
{
	vec3 center = (a + b + c) / 3.f;
	//Perform backface culling
	float dotNV = math::dot( math::normalize(center), math::normalize(center - m_Frustum.GetPositionOS()));
	if (dotNV >= m_TriLevelDotLUT[level])
	{
		return TriNext::CULL;
	}

	//Perform Frustum culling
	if (frustumCull)
	{
		auto intersect = m_Frustum.ContainsTriVolume(a, b, c, m_HeightMultLUT[level]);
		//auto intersect = m_Frustum.ContainsTriangle(a, b, c);
		if (intersect == VolumeCheck::OUTSIDE) return TriNext::CULL;
		if (intersect == VolumeCheck::CONTAINS)//stop frustum culling -> all children are also inside the frustum
		{
			//check if new splits are allowed
			if (level >= m_MaxLevel)return TriNext::LEAF;
			//split according to distance
			float aDistSq = math::distanceSquared(a, m_Frustum.GetPositionOS());
			float bDistSq = math::distanceSquared(b, m_Frustum.GetPositionOS());
			float cDistSq = math::distanceSquared(c, m_Frustum.GetPositionOS());
			float splitDistSq = m_DistanceLUT[level] * m_DistanceLUT[level];
			if (std::fminf(aDistSq, std::fminf(bDistSq, cDistSq)) < splitDistSq) return TriNext::SPLIT;
			return TriNext::LEAF;
		}
	}
	//check if new splits are allowed
	if (level >= m_MaxLevel)return TriNext::LEAF;
	//split according to distance
	float aDistSq = math::distanceSquared(a, m_Frustum.GetPositionOS());
	float bDistSq = math::distanceSquared(b, m_Frustum.GetPositionOS());
	float cDistSq = math::distanceSquared(c, m_Frustum.GetPositionOS());
	float splitDistSq = m_DistanceLUT[level] * m_DistanceLUT[level];
	if (std::fminf(aDistSq, std::fminf(bDistSq, cDistSq)) < splitDistSq) return TriNext::SPLITCULL;
	return TriNext::LEAF;
}

void Triangulator::RecursiveTriangle(vec3 a, vec3 b, vec3 c, int16 level, bool frustumCull)
{
	TriNext next = SplitHeuristic(a, b, c, level, frustumCull);
	if (next == CULL) return;
	//check if subdivision is needed based on camera distance
	else if (next == SPLIT || next == SPLITCULL)
	{
		//find midpoints
		vec3 A = b + ((c - b)*0.5f);
		vec3 B = c + ((a - c)*0.5f);
		vec3 C = a + ((b - a)*0.5f);
		//make the distance from center larger according to planet radius
		A = A * m_Planet->GetRadius() / math::length(A);
		B = B * m_Planet->GetRadius() / math::length(B);
		C = C * m_Planet->GetRadius() / math::length(C);
		//Make 4 new triangles
		int16 nLevel = level + 1;
		RecursiveTriangle(a, B, C, nLevel, next == SPLITCULL);//Winding is inverted
		RecursiveTriangle(A, b, C, nLevel, next == SPLITCULL);//Winding is inverted
		RecursiveTriangle(A, B, c, nLevel, next == SPLITCULL);//Winding is inverted
		RecursiveTriangle(A, B, C, nLevel, next == SPLITCULL);
	}
	else //put the triangle in the buffer
	{
		m_Positions.push_back(PatchInstance((BYTE)level, a, b-a, c-a));
	}
}


} // namespace render
} // namespace et
