#pragma once


namespace fw {


//================
// Component Pool
//================


//--------------------
// ComponentPool::Get
//
// element access
//
template <typename TComponentType>
TComponentType& ComponentPool::Get(size_t const idx)
{
	ET_ASSERT(TComponentType::GetTypeIndex() == m_ComponentType);
	return *static_cast<TComponentType*>(At(idx));
}

//--------------------
// ComponentPool::Get
//
// const element access
//
template <typename TComponentType>
TComponentType const& ComponentPool::Get(size_t const idx) const
{
	ET_ASSERT(TComponentType::GetTypeIndex() == m_ComponentType);
	return *static_cast<TComponentType const*>(At(idx));
}

//------------------------
// ComponentPool::Append
//
// Add an component to the end of the buffer
//
template<typename TComponentType, typename std::enable_if_t<!std::is_pointer<TComponentType>::value>*>
void ComponentPool::Append(TComponentType const& component)
{
	ET_ASSERT(TComponentType::GetTypeIndex() == m_ComponentType);
	Append(&component);
}


} // namespace fw
