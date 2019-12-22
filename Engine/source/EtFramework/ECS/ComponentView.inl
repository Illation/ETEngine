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

//-------------------------
// ComponentView::Declare
//
// initialize a read access to a parents components
//
template<typename TComponentType>
void ComponentView::Declare(ParentRead<TComponentType>& parentRead)
{
	m_ParentAccessors.push_back(Accessor(reinterpret_cast<void**>(&(parentRead.m_Data)), TComponentType::GetTypeIndex(), true));
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
	return ComponentSignature(temp.GetTypeList());
}


} // namespace framework


