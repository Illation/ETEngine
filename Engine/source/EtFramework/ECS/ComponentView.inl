#pragma once


namespace framework {


//================
// Component View
//================


//-------------------------
// ComponentView::Declare
//
// initialize a read access
//
template<typename TComponentType>
void ComponentView::Declare(ReadAccess<TComponentType>& read)
{
	m_Accessors.push_back(Accessor(reinterpret_cast<void**>(&(read.m_Data)), TComponentType::GetTypeIndex(), true));
}

//-------------------------
// ComponentView::Declare
//
// initialize a write access
//
template<typename TComponentType>
void ComponentView::Declare(WriteAccess<TComponentType>& write)
{
	m_Accessors.push_back(Accessor(reinterpret_cast<void**>(&(write.m_Data)), TComponentType::GetTypeIndex(), false));
}


// signature generation
//----------------------

//-------------------
// SignatureFromView
//
template<typename TViewType>
ComponentSignature SignatureFromView()
{
	TViewType temp;
	temp.Register();

	return ComponentSignature(temp.GetTypeList());
}


} // namespace framework


