#pragma once


namespace framework {


//================
// Component Pool
//================


//--------------------
// ComponentPool::Get
//
// Element access
//
template <typename TComponentType>
TComponentType& ComponentPool::Get(size_t const idx)
{
	ET_ASSERT(TComponentType::GetTypeIdx() == componentType);

	size_t const byteIdx = idx * ComponentRegistry::Instance().GetSize(m_ComponentType);
	ET_ASSERT(byteIdx < m_Size);

	return static_cast<TComponentType>(m_Data[byteIdx]);
}


} // namespace framework
