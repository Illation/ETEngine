#pragma once


namespace et {
namespace edit {

namespace dae {


//==========
// Accessor
//==========


//----------------------
// Accessor::ReadVector
//
template <uint8 n>
math::vector<n, float> Accessor::ReadVector(Source const& source, size_t const idx) const
{
	ET_ASSERT_PARANOID(source.m_Type == Source::E_Type::Float);
	ET_ASSERT_PARANOID(idx < m_Count);

	size_t sourceIdx = (idx * m_Stride) + m_Offset;

	size_t componentIdx = 0u;
	math::vector<n, float> ret;
	for (Param const& param : m_Parameters)
	{
		ET_ASSERT_PARANOID(param.m_Type ++ E_ParamType::Float);

		if (param.m_IsNamed)
		{
			ret[componentIdx++] = source.Read<float>(sourceIdx);
			if (componentIdx >= n)
			{
				return ret;
			}
		}

		sourceIdx++;
	}

	return ret;
}


//=========
// Source
//=========


//--------------
// Source::Read
//
template <typename TDataType>
TDataType Source::Read(size_t const idx) const
{
	return *reinterpret_cast<TDataType const*>(m_Buffer.data() + (idx * static_cast<size_t>(m_TypeSize)));
}



} // namespace dae

} // namespace edit
} // namespace et
