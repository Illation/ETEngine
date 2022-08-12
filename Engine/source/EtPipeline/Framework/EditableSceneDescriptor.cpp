#include <EtFramework/stdafx.h>
#include "EditableSceneDescriptor.h"

#include <EtCore/Reflection/JsonDeserializer.h>
#include <EtCore/Reflection/BinarySerializer.h>

#include <EtPipeline/Core/PipelineCtx.h>
#include <EtPipeline/Core/Content/FileResourceManager.h>


namespace et {
namespace pl {


//=================================
// Editable Scene Descriptor Asset
//=================================


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableSceneDescriptorAsset, "editable scene descriptor asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableSceneDescriptorAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableSceneDescriptorAsset) // force the asset class to be linked as it is only used in reflection


//----------------------------------------------
// EditableSceneDescriptorAsset::LoadFromMemory
//
bool EditableSceneDescriptorAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	fw::SceneDescriptor* const sceneDesc = new fw::SceneDescriptor();

	// convert that data to a string and deserialize it as json
	core::JsonDeserializer deserializer;
	if (!deserializer.DeserializeFromData(data, *sceneDesc))
	{
		ET_LOG_E(ET_CTX_PIPELINE, "EditableSceneDescriptorAsset::LoadFromMemory > Failed to deserialize descriptor!");
		delete sceneDesc;
		return false;
	}

	SetData(sceneDesc);
	return true;
}

//------------------------------------------------
// EditableSceneDescriptorAsset::GenerateInternal
//
bool EditableSceneDescriptorAsset::GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath)
{
	ET_UNUSED(dbPath);

	ET_ASSERT(m_RuntimeAssets.size() == 1u);
	m_RuntimeAssets[0].m_HasGeneratedData = true; // we set this to true either way because we don't want to write json to the package if generation fails

	// we ensure that no deserialized asset pointers load their data
	pl::FileResourceManager* const resMan = static_cast<pl::FileResourceManager*>(core::ResourceManager::Instance());
	bool const wasEnabled = resMan->IsLoadEnabled();
	if (wasEnabled)
	{
		resMan->SetLoadEnabled(false);
	}

	fw::SceneDescriptor sceneDesc;

	// deserialize from JSON
	std::vector<uint8> const& loadData = m_Asset->GetLoadData();
	core::JsonDeserializer deserializer;
	if (!deserializer.DeserializeFromData(loadData, sceneDesc))
	{
		ET_LOG_W(ET_CTX_PIPELINE, "EditableSceneDescriptorAsset::GenerateInternal > Failed to deserialize descriptor from json!");

		if (wasEnabled)
		{
			resMan->SetLoadEnabled(true);
		}

		return false;
	}

	// we can strip editor only components here in the future, or do other processing on components before they are loaded for runtime

	// serialize to binary
	bool const isVerbose = (buildConfig.m_Configuration != pl::BuildConfiguration::E_Configuration::Shipping);
	core::BinarySerializer serializer(isVerbose);

	std::vector<uint8>& writeData = m_RuntimeAssets[0].m_GeneratedData;
	if (!serializer.SerializeToData(sceneDesc, writeData))
	{
		writeData.clear();
		ET_LOG_W(ET_CTX_PIPELINE, "EditableSceneDescriptorAsset::GenerateInternal > Failed to serialize descriptor to binary!");

		if (wasEnabled)
		{
			resMan->SetLoadEnabled(true);
		}

		return false;
	}

	// make sure asset pointers can load again as per usual
	if (wasEnabled)
	{
		resMan->SetLoadEnabled(true);
	}

	return true;
}


} // namespace pl
} // namespace et
