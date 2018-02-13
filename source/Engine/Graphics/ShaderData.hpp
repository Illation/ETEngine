#pragma once
#include "../staticDependancies/glad/glad.h"
#include "../Helper/Hash.h"//Everything using shaders needs hash functionality
#include <string>
#include <vector>

class AbstractUniform
{
public:
	virtual const std::type_info& GetType() const = 0;

	GLint location = -1;
	std::string name;
};
template<typename T>
class Uniform;
namespace detail
{
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
		if (!(rhs == data))
		{
			data = rhs;
			detail::UploadUniform(*this);
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

	std::string GetName() { return m_Name; }

	template<typename T>
	bool Upload(uint32 uniform, const T &data)const
	{
		auto it = m_Uniforms.find(uniform);
		if (!(it == m_Uniforms.end()))
		{
			AbstractUniform* uni = it->second;
			assert(uni->GetType() == typeid(T));
			static_cast<Uniform<T>*>(uni)->Upload(data);
			return true;
		}
		assert(false);
		return false;
	}
private:
	friend class ShaderLoader;

	GLuint m_ShaderProgram;

	std::string m_Name;

	std::map<uint32, AbstractUniform*> m_Uniforms;
};