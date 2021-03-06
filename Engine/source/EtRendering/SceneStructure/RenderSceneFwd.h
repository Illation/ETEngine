#pragma once
#include "MaterialCollection.h"
#include "Light.h"


namespace et {
namespace render {


typedef core::slot_map<MaterialCollection::Mesh>::id_type T_MeshId;
typedef core::slot_map<MaterialCollection::MaterialInstance>::id_type T_MaterialInstanceId;
typedef core::slot_map<MaterialCollection>::id_type T_CollectionId;


} // namespace render
} // namespace et
