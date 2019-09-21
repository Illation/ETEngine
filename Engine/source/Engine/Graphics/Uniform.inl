#pragma once


//===================
// Uniform
//===================


//-------------------------------
// Uniform::Upload
//
// Upload the templated uniform to the GPU
//
template<typename T>
void Uniform<T>::Upload(const T &rhs)
{
	if (!(rhs == data))
	{
		data = rhs;
		Viewport::GetCurrentApiContext()->UploadUniform(*this);
	}
}

//-------------------------------
// Uniform::Init
//
// Sets the initial value of a uniform based on what the GPU holds
//
template<typename T>
void Uniform<T>::Init(uint32 const program)
{
	Viewport::GetCurrentApiContext()->InitUniform(program, *this);
}