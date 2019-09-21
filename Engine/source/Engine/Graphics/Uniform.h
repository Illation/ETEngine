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

#include "Uniform.inl"