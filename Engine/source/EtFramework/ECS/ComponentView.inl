#pragma once


namespace framework {


//================
// Component View
//================


//-------------------------
// ComponentView::Register
//
// initialize a read access
//
template<typename TComponentType>
void ComponentView::Register(ReadAccess<TComponentType>& read)
{
	m_Accessors.emplace_back(&read, TComponentType::GetTypeIndex(), true);
}

//-------------------------
// ComponentView::Register
//
// initialize a write access
//
template<typename TComponentType>
void ComponentView::Register(WriteAccess<TComponentType>& write)
{
	m_Accessors.emplace_back(&write, TComponentType::GetTypeIndex(), false);
}


} // namespace framework


