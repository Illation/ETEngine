#include "stdafx.h"
#include "TextureData.h"

#include "Shader.h"

#include <Engine/GraphicsHelper/PrimitiveRenderer.h>


TextureData::TextureData(GLuint handle, int32 width, int32 height, int32 depth):
	m_Handle(handle),
	m_Width(width),
	m_Height(height),
	m_Depth( depth )
{
}

TextureData::TextureData( int32 width, int32 height, int32 internalFormat, GLenum format, GLenum type, int32 depth ) :
	m_Width( width ),
	m_Height( height ),
	m_InternalFormat( internalFormat ),
	m_Format( format ),
	m_Type( type ), 
	m_Depth( depth )
{
	glGenTextures( 1, &m_Handle );
	ZeroMemory( &m_Parameters, sizeof( TextureParameters ) ); //ensure setting them when its called
}

TextureData::~TextureData()
{
	glDeleteTextures(1, &m_Handle);
}

void TextureData::Build( void* data /*= NULL*/ )
{
	if (m_Depth == 1)
	{
		STATE->BindTexture( GL_TEXTURE_2D, m_Handle );
		glTexImage2D(GL_TEXTURE_2D, 0, m_InternalFormat, m_Width, m_Height, 0, m_Format, m_Type, data );
	}
	else
	{
		STATE->BindTexture( GL_TEXTURE_3D, m_Handle );
		glTexImage3D(GL_TEXTURE_3D, 0, m_InternalFormat, m_Width, m_Height, m_Depth, 0, m_Format, m_Type, data );
	}
}

void TextureData::SetParameters( TextureParameters params )
{
	GLenum target = GetTarget();
	STATE->BindTexture(target, m_Handle );
	if(m_Parameters.minFilter != params.minFilter)
	{
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, params.minFilter );
	}
	if(m_Parameters.magFilter != params.magFilter)
	{
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, params.magFilter );
	}
	if(m_Parameters.wrapS != params.wrapS)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_S, params.wrapS );
	}
	if(m_Parameters.wrapT != params.wrapT)
	{
		glTexParameteri(target, GL_TEXTURE_WRAP_T, params.wrapT );
	}
	if(!etm::nearEqualsV(m_Parameters.borderColor, params.borderColor ))
	{
		glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, params.borderColor.data.data() );
	}
	if(m_Parameters.genMipMaps == false && params.genMipMaps == true)
	{
		glGenerateMipmap(target);
	}
	if(params.isDepthTex && m_Parameters.compareMode != params.compareMode)
	{
		glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, params.compareMode );//shadow map comp mode
	}
	if (m_Depth > 1)
	{
		if (m_Parameters.wrapR != params.wrapR)
		{
			glTexParameteri(target, GL_TEXTURE_WRAP_R, params.wrapR);
		}
	}
	m_Parameters = params;
}

bool TextureData::Resize( ivec2 newSize )
{
	if(newSize.x > m_Width || newSize.y > m_Height)
	{
		m_Width = newSize.x; m_Height = newSize.y;
		glDeleteTextures(1, &m_Handle);
		glGenTextures( 1, &m_Handle );
		Build();
		auto tempParams = m_Parameters;
		ZeroMemory( &m_Parameters, sizeof( TextureParameters ) ); //ensure setting them when its called
		SetParameters( tempParams );
		return true;
	}
	m_Width = newSize.x; m_Height = newSize.y;
	Build();
	return false;
}

CubeMap* EquirectangularToCubeMap(TextureData* pEqui, int32 resolution)
{
	//Create framebuffer
	GLuint captureFBO, captureRBO;
	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	STATE->BindFramebuffer(captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, resolution, resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

	//Preallocate memory for cubemap
	GLuint envCubemap;
	glGenTextures(1, &envCubemap);
	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (uint32 i = 0; i < 6; ++i)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, resolution, resolution, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	std::vector<mat4> captureViews = CubeCaptureViews();

	//Get the shader
	auto equiCubeShader = ContentManager::Load<ShaderData>("Shaders/FwdEquiCubeShader.glsl");

	// convert HDR equirectangular environment map to cubemap equivalent
	STATE->SetShader(equiCubeShader);
	glUniform1i(glGetUniformLocation(equiCubeShader->GetProgram(), "equirectangularMap"), 0);
	STATE->LazyBindTexture(0, GL_TEXTURE_2D, pEqui->GetHandle());
	glUniformMatrix4fv(glGetUniformLocation(equiCubeShader->GetProgram(), "projection"), 1, GL_FALSE, etm::valuePtr(CubeCaptureProjection()));

	//render the cube
	//***************

	STATE->SetViewport(ivec2(0), ivec2(resolution));
	STATE->BindFramebuffer(captureFBO);
	for (uint32 i = 0; i < 6; ++i)
	{
		glUniformMatrix4fv(glGetUniformLocation(equiCubeShader->GetProgram(), "view"), 1, GL_FALSE, etm::valuePtr(captureViews[i]));
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		PrimitiveRenderer::GetInstance()->Draw<primitives::Cube>();
	}
	STATE->BindFramebuffer(0);

	STATE->BindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);// #note might need to be rebound later
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	int32 mipNum = 1 + (int32)floor(log10((float)resolution) / log10(2.0));

	STATE->BindTexture(GL_TEXTURE_2D, 0);
	STATE->SetViewport(ivec2(0), WINDOW.Dimensions);

	glDeleteRenderbuffers(1, &captureRBO);
	glDeleteFramebuffers(1, &captureFBO);
	
	return new CubeMap(envCubemap, resolution, resolution, mipNum);
}

mat4 CubeCaptureProjection()
{
	return etm::perspective(etm::radians(90.0f), 1.0f, 0.1f, 10.0f);
}

std::vector<mat4> CubeCaptureViews()
{
	std::vector<mat4> ret;
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(1.0f,  0.0f,  0.0f), vec3(0.0f, -1.0f,  0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, -1.0f, 0.0f)));
	ret.push_back(etm::lookAt(vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), vec3(0.0f, -1.0f, 0.0f)));
	return ret;
}
