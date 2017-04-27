#include "stdafx.hpp"
#include "Triangulator.h"

#include "../Graphics/Frustum.h"
#include "Planet.h"

#include "../Components/TransformComponent.hpp"
#include "../Components/CameraComponent.hpp"

Triangulator::Triangulator(Planet* pPlanet)
	: m_pPlanet(pPlanet)
{
	m_pFrustum = new Frustum();
}
Triangulator::~Triangulator()
{
	SafeDelete(m_pFrustum);
}

void Triangulator::Init()
{
	auto ico = GetIcosahedronPositions(m_pPlanet->GetRadius());
	auto indices = GetIcosahedronIndices();
	for (size_t i = 0; i < indices.size(); i+=3)
	{
		m_Icosahedron.push_back(Tri(ico[indices[i]], ico[indices[i+1]], ico[indices[i+2]], nullptr, 0));
	}

	Precalculate();
	
	//First geometry generation
	GenerateGeometry();
}

bool Triangulator::Update()
{
	//Set Max subd level
	//bool levelChanged = false;
	//if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_UP))
	//{
	//	m_MaxLevel++;
	//	levelChanged = true;
	//}
	//if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_DOWN) && m_MaxLevel>0)
	//{
	//	m_MaxLevel--;
	//	levelChanged = true;
	//}
	//if (levelChanged)Precalculate();
	m_MaxLevel = 22;
	Precalculate();

	//Frustum update
	if (INPUT->IsKeyboardKeyPressed(SDL_SCANCODE_SPACE))m_LockFrustum = !m_LockFrustum;

	m_pFrustum->SetCullTransform(m_pPlanet->GetTransform()->GetWorld());
	if (!m_LockFrustum) m_pFrustum->SetToCamera(CAMERA);
	m_pFrustum->Update();

	return true;
}

void Triangulator::Precalculate()
{
	//determine culling angle behind planet based on max height
	float cullingAngle = acosf(m_pPlanet->GetRadius()/(m_pPlanet->GetRadius()+m_pPlanet->GetMaxHeight()));
	//Dot Product LUT
	m_TriLevelDotLUT.clear();
	m_TriLevelDotLUT.push_back(0.5f+sinf(cullingAngle));
	float angle = acosf(0.5f);
	for (int i = 1; i <= m_MaxLevel; i++)
	{
		angle *= 0.5f;
		m_TriLevelDotLUT.push_back(sinf(angle+cullingAngle));
	}
	//height multipliers
	m_HeightMultLUT.clear();
	glm::vec3 a = m_Icosahedron[0].a;
	glm::vec3 b = m_Icosahedron[0].b;
	glm::vec3 c = m_Icosahedron[0].c;
	glm::vec3 center = (a + b + c) / 3.f;
	center *= m_pPlanet->GetRadius() / glm::length(center);//+maxHeight
	m_HeightMultLUT.push_back(1 / glm::dot(glm::normalize(a), glm::normalize(center)));
	float normMaxHeight = m_pPlanet->GetMaxHeight() / m_pPlanet->GetRadius();
	for (int i = 1; i <= m_MaxLevel; i++)
	{
		glm::vec3 A = b + ((c - b)*0.5f);
		glm::vec3 B = c + ((a - c)*0.5f);
		c = a + ((b - a)*0.5f);
		a = A*m_pPlanet->GetRadius() / glm::length(A);
		b = B*m_pPlanet->GetRadius() / glm::length(B);
		c *= m_pPlanet->GetRadius() / glm::length(c);
		m_HeightMultLUT.push_back(1 / glm::dot(glm::normalize(a), glm::normalize(center)) + normMaxHeight);
	}
}

void Triangulator::GenerateGeometry()
{
	//Precalculate Distance LUT
	//The distances generated should keep the triangles smaller than m_AllowedTriPx at any level
	//In future only recalculate on FOV or triangle density change
	m_DistanceLUT.clear();
	float sizeL = glm::length(m_Icosahedron[0].a - m_Icosahedron[0].b);
	float frac = tanf((m_AllowedTriPx * glm::radians(m_pFrustum->GetFOV())) / WINDOW.Width);
	for (int level = 0; level < m_MaxLevel+5; level++)
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

TriNext Triangulator::SplitHeuristic(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, short level, bool frustumCull)
{
	glm::vec3 center = (a + b + c) / 3.f;
	//Perform backface culling
	float dotNV = glm::dot(glm::normalize(center), glm::normalize(center - m_pFrustum->GetPositionOS()));
	if (dotNV >= m_TriLevelDotLUT[level])
	{
		return TriNext::CULL;
	}

	//Perform Frustum culling
	if (frustumCull)
	{
		auto intersect = m_pFrustum->ContainsTriVolume(a, b, c, m_HeightMultLUT[level]);
		//auto intersect = m_pFrustum->ContainsTriangle(a, b, c);
		if (intersect == VolumeCheck::OUTSIDE) return TriNext::CULL;
		if (intersect == VolumeCheck::CONTAINS)//stop frustum culling -> all children are also inside the frustum
		{
			//check if new splits are allowed
			if (level >= m_MaxLevel)return TriNext::LEAF;
			//split according to distance
			float aDist = glm::length(a - m_pFrustum->GetPositionOS());
			float bDist = glm::length(b - m_pFrustum->GetPositionOS());
			float cDist = glm::length(c - m_pFrustum->GetPositionOS());
			if (std::fminf(aDist, std::fminf(bDist, cDist)) < m_DistanceLUT[level])return TriNext::SPLIT;
			return TriNext::LEAF;
		}
	}
	//check if new splits are allowed
	if (level >= m_MaxLevel)return TriNext::LEAF;
	//split according to distance
	float aDist = glm::length(a - m_pFrustum->GetPositionOS());
	float bDist = glm::length(b - m_pFrustum->GetPositionOS());
	float cDist = glm::length(c - m_pFrustum->GetPositionOS());
	if (std::fminf(aDist, std::fminf(bDist, cDist)) < m_DistanceLUT[level])return TriNext::SPLITCULL;
	return TriNext::LEAF;
}

void Triangulator::RecursiveTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, short level, bool frustumCull)
{
	TriNext next = SplitHeuristic(a, b, c, level, frustumCull);
	if (next == CULL) return;
	//check if subdivision is needed based on camera distance
	else if (next == SPLIT || next == SPLITCULL)
	{
		//find midpoints
		glm::vec3 A = b + ((c - b)*0.5f);
		glm::vec3 B = c + ((a - c)*0.5f);
		glm::vec3 C = a + ((b - a)*0.5f);
		//make the distance from center larger according to planet radius
		A *= m_pPlanet->GetRadius() / glm::length(A);
		B *= m_pPlanet->GetRadius() / glm::length(B);
		C *= m_pPlanet->GetRadius() / glm::length(C);
		//Make 4 new triangles
		short nLevel = level + 1;
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