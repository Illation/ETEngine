#include "stdafx.h"
#include "GltfDom.h"

#include <EtCore/IO/BinaryReader.h>


namespace et {
namespace edit {

namespace glTF {


void glTF::LogGLTFVersionSupport()
{
	ET_LOG_I(ET_CTX_EDITOR, "glTF minVersion '%f' maxVersion '%f'", glTF::minVersion, glTF::maxVersion);
}

bool glTF::OpenBufferViewReader(glTFAsset& asset, uint32 viewIdx, core::BinaryReader* pViewReader)
{
	if (viewIdx >= (int32)asset.dom.bufferViews.size())
	{
		ET_LOG_E(ET_CTX_EDITOR, "BufferView index out of range");
		return false;
	}

	BufferView& view = asset.dom.bufferViews[viewIdx];
	if (view.buffer >= (uint32)asset.dom.buffers.size())
	{
		ET_LOG_E(ET_CTX_EDITOR, "Buffer index out of range");
		return false;
	}

	Buffer& buffer = asset.dom.buffers[view.buffer];
	if (!buffer.uri.IsEvaluated())
	{
		if (!buffer.uri.Evaluate(asset.basePath))
		{
			ET_LOG_E(ET_CTX_EDITOR, "Failed to evaluate buffer URI");
			return false;
		}
	}

	if (buffer.uri.GetType() == core::URI::E_Type::None)
	{
		if (view.buffer >= static_cast<uint32>(asset.dataChunks.size()))
		{
			ET_LOG_E(ET_CTX_EDITOR, "No data chunk loaded for glb buffer");
			return false;
		}

		pViewReader->Open(asset.dataChunks[view.buffer].chunkData, static_cast<size_t>(view.byteOffset), static_cast<size_t>(view.byteLength));
	}
	else
	{
		pViewReader->Open(buffer.uri.GetEvaluatedData(), static_cast<size_t>(view.byteOffset), static_cast<size_t>(view.byteLength));
	}

	if (!pViewReader->Exists())
	{
		ET_LOG_E(ET_CTX_EDITOR, "glTF Failed to read the buffer view!");
		return false;
	}

	return true;
}

bool glTF::GetAccessorData(glTFAsset& asset, uint32 idx, std::vector<uint8>& data)
{
	if (idx >= (uint32)asset.dom.accessors.size())
	{
		ET_LOG_E(ET_CTX_EDITOR, "Accessor index out of range");
		return false;
	}

	Accessor& accessor = asset.dom.accessors[idx];
	if (accessor.sparse || accessor.bufferView == -1)
	{
		ET_LOG_E(ET_CTX_EDITOR, "Unsupported accessor type, sparse accessors are not yet implemented");
		return false;
	}

	if (accessor.bufferView >= (int32)asset.dom.bufferViews.size())
	{
		ET_LOG_E(ET_CTX_EDITOR, "BufferView index out of range");
		return false;
	}

	BufferView& view = asset.dom.bufferViews[accessor.bufferView];
	if (view.buffer >= (uint32)asset.dom.buffers.size())
	{
		ET_LOG_E(ET_CTX_EDITOR, "Buffer index out of range");
		return false;
	}

	uint8 compSize = ComponentTypes[accessor.componentType];
	uint8 compsPerEl = AccessorTypes[accessor.type].first;
	uint8 elSize = compSize * compsPerEl;

	//Validation
	if (!(accessor.byteOffset % compSize == 0))
	{
		ET_LOG_W(ET_CTX_EDITOR, "Accessors byte offset needs to be a multiple of the component size");
	}

	if (accessor.min.size())
	{
		if (!((uint32)accessor.min.size() == (uint32)compsPerEl))
		{
			ET_LOG_W(ET_CTX_EDITOR, "Accessors min array size must equal components per element");
		}
	}

	if (accessor.max.size())
	{
		if (!((uint32)accessor.max.size() == (uint32)compsPerEl))
		{
			ET_LOG_W(ET_CTX_EDITOR, "Accessors max array size must equal components per element");
		}
	}

	uint32 stride = (view.byteStride == -1) ? (uint32)elSize : view.byteStride;
	if (!(stride % compSize == 0))
	{
		ET_LOG_W(ET_CTX_EDITOR, "Accessors byte stride needs to be a multiple of the component size");
	}

	if ((accessor.byteOffset + stride * (accessor.count - 1) + elSize) > view.byteLength)
	{
		ET_LOG_W(ET_CTX_EDITOR, "Accessors doesn't fit buffer view");
	}

	core::BinaryReader* pViewReader = new core::BinaryReader();
	if (!OpenBufferViewReader(asset, accessor.bufferView, pViewReader))
	{
		delete pViewReader;
		ET_LOG_E(ET_CTX_EDITOR, "Unable to read buffer view");
		return false;
	}

	for (uint64 i = static_cast<uint64>(accessor.byteOffset);
		i < static_cast<uint64>(accessor.byteOffset) + (stride * accessor.count);
		i += static_cast<uint64>(stride))
	{
		pViewReader->SetBufferPosition(static_cast<size_t>(i));
		for (uint32 j = 0; j < elSize; ++j)
		{
			data.push_back(pViewReader->Read<uint8>());
		}
	}

	delete pViewReader;
	return true;
}


} // namespace glTF

} // namespace edit
} // namespace et
