#pragma once
#include "../Graphics/FrameBuffer.hpp"

class Planet;

class Atmosphere
{
public:
	Atmosphere(Planet* pPlanet);
	~Atmosphere();

	void Precalculate();
	void Initialize();
	void Draw();
	
private:
	//Parameters
	Planet* m_pPanet;

	//Camera and pos reconstruction from gbuffer
	GLint m_uCamPos;
	GLint m_uProjA;
	GLint m_uProjB;
	GLint m_uViewProjInv;

	ShaderData* m_pShader;
};

