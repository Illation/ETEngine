#pragma once


namespace et {
namespace core {


//===================
// High Res Duration
//===================


//-----------------------
// HighResDuration::Cast
//
// Return the timestamp as a different data type
//
template <typename TDataType>
TDataType HighResDuration::Cast() const
{
#ifdef ET_PLATFORM_LINUX
	return static_cast<TDataType>(m_Impl.tv_sec) + (static_cast<TDataType>(m_Impl.tv_nsec) / 1000000000);
#else
	return static_cast<TDataType>(std::chrono::duration_cast<std::chrono::nanoseconds>(m_Impl).count()) * 1e-9f;
#endif
}


} // namespace core
} // namespace et

