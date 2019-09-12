#pragma once


// forward declarations
template<typename T>
class Uniform;


//---------------------------------
// I_Uniform
//
// Interface for using  uniforms
//
class I_Uniform
{
public:
	virtual const std::type_info& GetType() const = 0;
	virtual void Init(uint32 const program) = 0;

	GLint location = -1;
	std::string name;
};


//---------------------------------
// Uniform
//
// Specific templated uniform
//
template<typename T>
class Uniform : public I_Uniform
{
public:
	virtual const std::type_info& GetType() const { return typeid(T); }
	void Upload(const T &rhs);
	void Init(uint32 const program);
	T data;
};

// implementation for uploading a uniform to the currently bound shader
namespace detail
{
	void UploadUniform(const Uniform<bool> &uniform);
	void UploadUniform(const Uniform<int32> &uniform);
	void UploadUniform(const Uniform<uint32> &uniform);
	void UploadUniform(const Uniform<float> &uniform);
	void UploadUniform(const Uniform<vec2> &uniform);
	void UploadUniform(const Uniform<vec3> &uniform);
	void UploadUniform(const Uniform<vec4> &uniform);
	void UploadUniform(const Uniform<mat3> &uniform);
	void UploadUniform(const Uniform<mat4> &uniform);

	void InitUniform(uint32 const program, Uniform<bool> &uniform);
	void InitUniform(uint32 const program, Uniform<int32> &uniform);
	void InitUniform(uint32 const program, Uniform<uint32> &uniform);
	void InitUniform(uint32 const program, Uniform<float> &uniform);
	void InitUniform(uint32 const program, Uniform<vec2> &uniform);
	void InitUniform(uint32 const program, Uniform<vec3> &uniform);
	void InitUniform(uint32 const program, Uniform<vec4> &uniform);
	void InitUniform(uint32 const program, Uniform<mat3> &uniform);
	void InitUniform(uint32 const program, Uniform<mat4> &uniform);
}

#include "Uniform.inl"