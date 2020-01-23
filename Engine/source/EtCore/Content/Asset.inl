#pragma once


namespace et {
namespace core {


//---------------------------------
// RawAsset::UnloadInternal
//
// Unload an asset by calling delete on its data
//
template <class T_DataType>
void RawAsset<T_DataType>::UnloadInternal()
{
	if (m_Data != nullptr)
	{
		delete m_Data;
		m_Data = nullptr;
	}
	m_LoadData.clear();
}


} // namespace core
} // namespace et
