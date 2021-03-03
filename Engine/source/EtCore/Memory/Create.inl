#pragma once


namespace et {


//========
// Create
//========


//-----------------
// Create::c-tor
//
// Move from other create
//
template <typename TDataType>
Create<TDataType>::Create(Create&& moving)
	: m_Ptr(moving.m_Ptr)
{
	moving.m_Ptr = nullptr;
}

//-------------------
// Create::operator=
//
// Move from other create
//
template <typename TDataType>
Create<TDataType>& Create<TDataType>::operator=(Create&& moving)
{
	m_Ptr = moving.m_Ptr;
	moving.m_Ptr = nullptr;
}

//-----------------
// Create::d-tor
//
// If noone has released the created memory, we delete the object
//
template <typename TDataType>
Create<TDataType>::~Create()
{
	delete m_Ptr;
}

//-----------------
// Create::Release
//
// Hand memory over to caller
//
template <typename TDataType>
TDataType* Create<TDataType>::Release()
{
	TDataType* const temp = m_Ptr;
	m_Ptr = nullptr;
	return temp;
}


} // namespace et
