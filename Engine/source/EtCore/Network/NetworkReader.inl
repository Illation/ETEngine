#pragma once

namespace et {
namespace core {


//---------------------
// NetworkReader::Read
//
template<typename TDataType>
TDataType NetworkReader::Read()
{
	TDataType const ret = m_Reader.Read<TDataType>();
	return m_SwapByteOrder ? SwapEndian(ret) : ret;
}


} // namespace core
} // namespace et

