#pragma once



namespace et {
namespace edit {

namespace glTF {


template<typename T>
bool GetAccessorScalarArray(glTFAsset& asset, uint32 idx, std::vector<T>& data)
{
	if (idx >= (uint32)asset.dom.accessors.size())
	{
		ET_LOG_E(ET_CTX_EDITOR, "Accessor index out of range");
		return false;
	}

	Accessor& accessor = asset.dom.accessors[idx];
	uint8 compsPerEl = AccessorTypes[accessor.type].first;
	std::vector<uint8> accessorData;
	if (!GetAccessorData(asset, idx, accessorData))
	{
		ET_LOG_W(ET_CTX_EDITOR, "Unable to get accessor data");
	}

	core::BinaryReader binReader;
	binReader.Open(accessorData);
	if (!(binReader.Exists()))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Unable to convert accessor data");
		return false;
	}

	for (uint32 i = 0; i < accessor.count * compsPerEl; ++i)
	{
		if (binReader.GetBufferPosition() >= (int32)accessorData.size())
		{
			ET_LOG_E(ET_CTX_EDITOR, "Binary reader out of range");
			return false;
		}

		switch (accessor.componentType)
		{
		case ComponentType::BYTE:
			data.push_back(static_cast<T>(binReader.Read<int8>()));
			break;
		case ComponentType::UNSIGNED_BYTE:
			data.push_back(static_cast<T>(binReader.Read<uint8>()));
			break;
		case ComponentType::SHORT:
			data.push_back(static_cast<T>(binReader.Read<int16>()));
			break;
		case ComponentType::UNSIGNED_SHORT:
			data.push_back(static_cast<T>(binReader.Read<uint16>()));
			break;
		case ComponentType::UNSIGNED_INT:
			data.push_back(static_cast<T>(binReader.Read<uint32>()));
			break;
		case ComponentType::FLOAT:
			data.push_back(static_cast<T>(binReader.Read<float>()));
			break;
		}
	}

	return true;
}

template <uint8 n, class T>
bool GetAccessorVectorArray(glTFAsset& asset, uint32 idx, std::vector<math::vector<n, T>>& data, bool convertCoords)
{
	if (idx >= (uint32)asset.dom.accessors.size())
	{
		ET_LOG_E(ET_CTX_EDITOR, "Accessor index out of range");
		return false;
	}

	Accessor& accessor = asset.dom.accessors[idx];
	uint8 compsPerEl = AccessorTypes[accessor.type].first;
	if (compsPerEl != n)
	{
		ET_LOG_E(ET_CTX_EDITOR, "Accessor type mismatch with vector size");
		return false;
	}

	std::vector<T> scalars;
	if (!GetAccessorScalarArray(asset, idx, scalars))
	{
		ET_LOG_E(ET_CTX_EDITOR, "Unable to get accessor scalar array for vector array");
		return false;
	}

	if (convertCoords && n != 3)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Converting coordinates of a non-3D vector");
	}

	convertCoords &= n > 1;
	for (uint32 i = 0; i < scalars.size() / n; ++i)
	{
		math::vector<n, T> vec;
		for (uint32 j = 0; j < n; ++j)
		{
			vec[(uint8)j] = scalars[i*(uint32)n + j];
		}

		if (convertCoords)
		{
			vec[1] = -vec[1];
		}

		data.push_back(vec);
	}

	return true;
}


} // namespace glTF

} // namespace edit
} // namespace et
