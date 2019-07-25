#pragma once


//---------------------------------
// Asset::Unload
//
// Unload an asset by calling delete on its data
//
template <class T>
void Asset<T>::Unload()
{
	if (m_Data != nullptr)
	{
		delete m_Data;
		m_Data = nullptr;
	}
}