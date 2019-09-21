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