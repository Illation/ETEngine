#pragma once
#include "../staticDependancies/glad/glad.h"
#include <string>
#include <vector>

class AbstractUniform
{
public:
	virtual const std::type_info& GetType() const = 0;

	GLint location;
};
template<typename T>
class Uniform;
namespace detail
{
	template<typename T>
	void UploadUniform(const Uniform<T> &uniform)
	{
		//Template specialization for uploading
		std::cout << "attempted to upload unspecialized uniform to shader" << std::endl;
	}
	void UploadUniform(const Uniform<bool> &uniform);
	void UploadUniform(const Uniform<int32> &uniform);
	void UploadUniform(const Uniform<float> &uniform);
	void UploadUniform(const Uniform<vec2> &uniform);
	void UploadUniform(const Uniform<vec3> &uniform);
	void UploadUniform(const Uniform<vec4> &uniform);
	void UploadUniform(const Uniform<mat3> &uniform);
	void UploadUniform(const Uniform<mat4> &uniform);
}
template<typename T>
class Uniform : public AbstractUniform
{
public:
	virtual const std::type_info& GetType() const { return typeid(T); }
	void Upload(const T &rhs)
	{
		if (rhs != data)
		{
			data = rhs;
			detail::UploadUniform(this);
		}
	}
	T data;
};


class ShaderData
{
public:
	ShaderData(GLuint shaderProg);
	~ShaderData();

	GLuint GetProgram() { return m_ShaderProgram; }

	string GetName() { return m_Name; }
private:
	friend class ShaderLoader;

	GLuint m_ShaderProgram;

	string m_Name;

	std::map<uint32, AbstractUniform*> m_Uniforms;
};

