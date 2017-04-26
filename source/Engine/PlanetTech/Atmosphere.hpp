#pragma once
#include "../Graphics/FrameBuffer.hpp"

class Planet;

class Atmosphere : public FrameBuffer
{
public:
	Atmosphere(Planet* pPlanet);
	~Atmosphere();

	void Precalculate();
	
private:
	void AccessShaderAttributes();
	void UploadDerivedVariables();

	//Parameters
	Planet* m_pPanet;

	//Camera and pos reconstruction from gbuffer
	GLint m_uCamPos;
	GLint m_uProjA;
	GLint m_uProjB;
	GLint m_uViewProjInv;
};

