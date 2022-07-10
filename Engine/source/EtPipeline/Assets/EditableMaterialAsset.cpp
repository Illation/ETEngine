#include "stdafx.h"
#include "EditableMaterialAsset.h"

#include <EtCore/Reflection/JsonDeserializer.h>
#include <EtCore/Reflection/BinarySerializer.h>

#include <EtRendering/MaterialSystem/MaterialDescriptor.h>


namespace et {
namespace pl {


//--------------------------------------
// ConvertMaterialDescriptorToBinary
//
// convert a JSON string (in uint8[] form) to serialized binary data
//
bool ConvertMaterialDescriptorToBinary(std::vector<uint8> const& jsonData, BuildConfiguration const &buildConfig, std::vector<uint8> &binData)
{
	render::MaterialDescriptor descriptor;

	// deserialize from JSON
	core::JsonDeserializer deserializer;
	if (!deserializer.DeserializeFromData(jsonData, descriptor))
	{
		LOG("ConvertMaterialDescriptorToBinary > Failed to deserialize descriptor from json!", core::LogLevel::Warning);
		return false;
	}

	// serialize to binary
	bool const isVerbose = (buildConfig.m_Configuration != pl::BuildConfiguration::E_Configuration::Shipping);
	core::BinarySerializer serializer(isVerbose);
	if (!serializer.SerializeToData(descriptor, binData))
	{
		binData.clear();
		LOG("ConvertMaterialDescriptorToBinary > Failed to serialize descriptor to binary!", core::LogLevel::Warning);
		return false;
	}

	return true;
}


// reflection
RTTR_REGISTRATION
{
	BEGIN_REGISTER_CLASS(EditableMaterialAsset, "editable material asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableMaterialAsset, EditorAssetBase);

	BEGIN_REGISTER_CLASS(EditableMaterialInstanceAsset, "editable material instance asset")
	END_REGISTER_CLASS_POLYMORPHIC(EditableMaterialInstanceAsset, EditorAssetBase);
}
DEFINE_FORCED_LINKING(EditableMaterialAsset) // force the asset class to be linked as it is only used in reflection


//=========================
// Editable Material Asset
//=========================


//---------------------------------------
// EditableMaterialAsset::LoadFromMemory
//
bool EditableMaterialAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	render::MaterialDescriptor descriptor;
	core::JsonDeserializer deserializer;
	if (!deserializer.DeserializeFromData(data, descriptor))
	{
		LOG("EditableMaterialAsset::LoadFromMemory > Failed to deserialize data from a JSON format into a material descriptor", core::LogLevel::Warning);
		return false;
	}

	SetData(render::MaterialAsset::CreateMaterial(m_Asset->GetReferences(), descriptor, static_cast<render::MaterialAsset*>(m_Asset)->m_DrawType));
	return (GetData() != nullptr);
}

//-----------------------------------------
// EditableMaterialAsset::GenerateInternal
//
bool EditableMaterialAsset::GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath)
{
	ET_UNUSED(dbPath);

	ET_ASSERT(m_RuntimeAssets.size() == 1u);
	m_RuntimeAssets[0].m_HasGeneratedData = true; // we set this to true either way because we don't want to write json to the package if generation fails

	std::vector<uint8> const& loadData = m_Asset->GetLoadData();
	std::vector<uint8>& writeData = m_RuntimeAssets[0].m_GeneratedData;

	return ConvertMaterialDescriptorToBinary(loadData, buildConfig, writeData);
}


//==================================
// Editable Material Instance Asset
//==================================


//-----------------------------------------------
// EditableMaterialInstanceAsset::LoadFromMemory
//
bool EditableMaterialInstanceAsset::LoadFromMemory(std::vector<uint8> const& data)
{
	render::MaterialDescriptor descriptor;
	core::JsonDeserializer deserializer;
	if (!deserializer.DeserializeFromData(data, descriptor))
	{
		LOG("EditableMaterialAsset::LoadFromMemory > Failed to deserialize data from a JSON format into a material descriptor", core::LogLevel::Warning);
		return false;
	}

	SetData(render::MaterialInstanceAsset::CreateMaterialInstance(m_Asset->GetReferences(), descriptor));
	return (GetData() != nullptr);
}

//-------------------------------------------------
// EditableMaterialInstanceAsset::GenerateInternal
//
bool EditableMaterialInstanceAsset::GenerateInternal(BuildConfiguration const& buildConfig, std::string const& dbPath)
{
	ET_UNUSED(dbPath);

	ET_ASSERT(m_RuntimeAssets.size() == 1u);
	m_RuntimeAssets[0].m_HasGeneratedData = true; 

	std::vector<uint8> const& loadData = m_Asset->GetLoadData();
	std::vector<uint8>& writeData = m_RuntimeAssets[0].m_GeneratedData;

	return ConvertMaterialDescriptorToBinary(loadData, buildConfig, writeData);
}


} // namespace pl
} // namespace et
