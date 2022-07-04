#pragma once


namespace et {
namespace core {


//======
// Time
//======


//---------------
// Time::GetTime
//
// Return the current elapsed time as a specific timestamp
//
template<typename TDataType>
TDataType Time::GetTime() const
{
	return (m_Last - m_Begin).Cast<TDataType>();
}


} // namespace core
} // namespace et
