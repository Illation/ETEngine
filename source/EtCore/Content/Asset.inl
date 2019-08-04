#pragma once


//---------------------------------
// RawAsset::Unload
//
// Unload an asset by calling delete on its data
//
template <class T_DataType>
void RawAsset<T_DataType>::Unload()
{
	if (m_Data != nullptr)
	{
		delete m_Data;
		m_Data = nullptr;
	}
}