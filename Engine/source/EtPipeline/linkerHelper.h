#pragma once
#include <EtPipeline/Assets/EditableAssetStub.h>
#include <EtPipeline/Assets/EditableAudioAsset.h>
#include <EtPipeline/Assets/EditableEnvironmentMap.h>
#include <EtPipeline/Assets/EditableFontAsset.h>
#include <EtPipeline/Assets/EditableSdfFont.h>
#include <EtPipeline/Assets/EditableFreetypeFont.h>
#include <EtPipeline/Assets/EditableGuiDocument.h>
#include <EtPipeline/Assets/EditableMaterialAsset.h>
#include <EtPipeline/Assets/EditableMeshAsset.h>
#include <EtPipeline/Assets/EditableSceneDescriptor.h>
#include <EtPipeline/Assets/EditableShaderAsset.h>
#include <EtPipeline/Assets/EditableTextureAsset.h>


namespace et {
namespace pl {


//---------------------------------
// ForceLinking
//
// Add classes here that the linker thinks wouldn't be used by this project but are in fact used by reflection
//
void ForceLinking()
{
	FORCE_LINKING(EditableStubAsset)
	FORCE_LINKING(EditableAudioAsset)
	FORCE_LINKING(EditableSceneDescriptorAsset)
	FORCE_LINKING(EditableEnvironmentMapAsset)
	FORCE_LINKING(EditableFontAsset)
	FORCE_LINKING(EditableSdfFontAsset)
	FORCE_LINKING(EditableFreetypeFontAsset)
	FORCE_LINKING(EditableGuiDocumentAsset)
	FORCE_LINKING(EditableMaterialAsset)
	FORCE_LINKING(EditableMeshAsset)
	FORCE_LINKING(EditableShaderAsset)
	FORCE_LINKING(EditableTextureAsset)
}


} // namespace pl
} // namespace et

