#include "stdafx.h"
#include "ColladaImporter.h"

#include <gtkmm/label.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/separator.h>
#include <gtkmm/box.h>

#include <EtCore/FileSystem/FileUtil.h>

#include <EtPipeline/Assets/EditableMeshAsset.h>
#include <EtPipeline/Import/MeshDataContainer.h>

#include <EtEditor/Util/GtkUtil.h>

#include "ColladaParser.h"


namespace et {
namespace edit {


//==================
// Collada Importer
//==================


//------------------------
// ColladaImporter::c-tor
//
ColladaImporter::ColladaImporter()
	: ImporterBase()
{
	m_SupportedExtensions.push_back("dae");
}

//-------------------------------
// ColladaImporter::SetupOptions
//
void ColladaImporter::SetupOptions(Gtk::Frame* const frame, T_SensitiveFn& sensitiveFn)
{
	Gtk::Box* const vbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_VERTICAL);
	frame->add(*vbox);

	// at least one asset type should be checked for the import buttonns to be active

	// create a checkbox and label combo
	auto makeOptionFn = 
		[vbox](Glib::ustring const& label, bool& option, bool const isSensitive) -> Gtk::CheckButton*
		{
			Gtk::Box* const hbox = Gtk::make_managed<Gtk::Box>(Gtk::ORIENTATION_HORIZONTAL);
			vbox->pack_start(*hbox, false, true, 3u);

			Gtk::CheckButton* const check = Gtk::make_managed<Gtk::CheckButton>();
			check->set_active(option);
			check->set_sensitive(isSensitive);
			check->signal_toggled().connect([&option, check]()
				{
					option = check->get_active();
				});

			hbox->pack_start(*check, false, false, 3u);
			hbox->pack_start(*Gtk::make_managed<Gtk::Label>(label), false, false, 3u);

			hbox->set_sensitive(isSensitive);

			return check;
		};

	vbox->pack_start(*Gtk::make_managed<Gtk::Label>("Asset types"), false, true, 3u);

	std::vector<Gtk::CheckButton*> assetButtons;

	assetButtons.push_back(makeOptionFn("Import Meshes", m_ImportMeshes, true));
	assetButtons.push_back(makeOptionFn("Import Skeletons", m_ImportSkeletons, false));
	assetButtons.push_back(makeOptionFn("Import Animations", m_ImportAnimations, false));

	for (Gtk::CheckButton* const checkButton : assetButtons)
	{
		checkButton->signal_toggled().connect([assetButtons, &sensitiveFn]()
			{
				sensitiveFn(std::find_if(assetButtons.cbegin(), assetButtons.cend(), [](Gtk::CheckButton const* const btn)
					{
						return btn->get_active();
					}) != assetButtons.cend());
			});
	}

	vbox->pack_start(*Gtk::make_managed<Gtk::Separator>(Gtk::ORIENTATION_HORIZONTAL), false, true, 3u);
	vbox->pack_start(*Gtk::make_managed<Gtk::Label>("Mesh options"), false, true, 3u);
	makeOptionFn("Calculate Tangent Space", m_CalculateTangentSpace, true);
	makeOptionFn("Pre Transform Vertices", m_PreTransformVertices, true);
	makeOptionFn("Remove duplicate vertices", m_RemoveDuplicateVertices, true);
	makeOptionFn("Include Skeletal data", m_IncludeSkeletalData, false);

	vbox->pack_start(*Gtk::make_managed<Gtk::Separator>(Gtk::ORIENTATION_HORIZONTAL), false, true, 3u);
}

//-------------------------
// ColladaImporter::Import
//
bool ColladaImporter::Import(std::vector<uint8> const& importData, std::string const& filePath, std::vector<pl::EditorAssetBase*>& outAssets) const
{
	UNUSED(importData);

	// read the barebones document
	ColladaParser const parser(importData);
	if (!parser.IsValid())
	{
		LOG("Collada parser didn't complete due to invalid document", core::LogLevel::Warning);
		return false;
	}
	
	// read components
	if (m_ImportMeshes)
	{
		std::vector<dae::Node> nodes;
		std::vector<dae::VisualScene> scenes;
		parser.IterateNodes([&nodes](core::XML::Element const& nodeEl, dae::Asset const&)
			{
				nodes.emplace_back();
				ColladaParser::ReadNode(nodes[nodes.size() - 1u], nodeEl);
			});

		parser.IterateVisualScenes([&scenes](core::XML::Element const& sceneEl, dae::Asset const&)
			{
				scenes.emplace_back();
				ColladaParser::ReadScene(scenes[scenes.size() - 1u], sceneEl);
			});

		std::vector<pl::MeshDataContainer> containers;
		parser.IterateGeometries([this, &containers, &nodes, &scenes](core::XML::Element const& geometryEl, dae::Asset const& asset)
		{
			core::XML::Element const* const meshEl = ColladaParser::GetMeshElFromGeometry(geometryEl);
			if (meshEl == nullptr)
			{
				return; // for now we only support mesh geometries
			}

			// read mesh and ensure we can use it
			//------------------------------------
			dae::Mesh mesh;
			if (!ColladaParser::ReadMesh(mesh, *meshEl))
			{
				return;
			}

			// figure out which inputs we actually want to use
			std::vector<size_t> inputIndices;
			for (size_t inputIdx = 0u; inputIdx < mesh.m_ResolvedInputs.size(); ++inputIdx)
			{
				switch (mesh.m_ResolvedInputs[inputIdx].m_Input.m_Semantic)
				{
				case dae::E_Semantic::Position:
				case dae::E_Semantic::Normal:
				case dae::E_Semantic::Binormal:
				case dae::E_Semantic::Tangent:
				case dae::E_Semantic::Color:
				case dae::E_Semantic::Texcoord:
					auto foundIt = std::find_if(inputIndices.begin(), inputIndices.end(), [inputIdx, &mesh](size_t const idx)
						{
							return (mesh.m_ResolvedInputs[idx].m_Input.m_Semantic == mesh.m_ResolvedInputs[inputIdx].m_Input.m_Semantic);
						});

					if (foundIt == inputIndices.cend())
					{
						inputIndices.push_back(inputIdx);
					}
					else if (mesh.m_ResolvedInputs[*foundIt].m_Input.m_Set > mesh.m_ResolvedInputs[inputIdx].m_Input.m_Set)
					{
						*foundIt = inputIdx;
					}

					break;
				}
			}

			// figure out transform of the mesh
			//----------------------------------
			mat4 meshTransform;
			bool hasNode = false;
			std::string nodeName;

			{
				core::HashString const meshId = ColladaParser::GetElementId(geometryEl);
				if (!meshId.IsEmpty())
				{
					for (dae::Node const& node : nodes)
					{
						if (node.GetGeometryTransformName(meshTransform, nodeName, meshId))
						{
							hasNode = true;
							break;
						}
					}

					for (dae::VisualScene const& scene : scenes)
					{
						if (hasNode)
						{
							break;
						}

						for (dae::Node const& node : scene.m_Nodes)
						{
							if (node.GetGeometryTransformName(meshTransform, nodeName, meshId))
							{
								hasNode = true;
								break;
							}
						}
					}
				}
			}

			mat3 stationary;
			if (hasNode)
			{
				meshTransform = math::inverse(meshTransform);
				stationary = math::CreateFromMat4(meshTransform);
			}

			bool const useTransform = hasNode && m_PreTransformVertices;

			// create mesh container
			//-----------------------
			containers.emplace_back();
			pl::MeshDataContainer& meshContainer = containers[containers.size() - 1u];

			size_t usedSet = dae::Input::s_InvalidIndex;
			size_t const increment = mesh.m_MaxInputOffset + 1u;
			for (size_t const inputIdx : inputIndices)
			{
				ET_ASSERT(mesh.m_ResolvedInputs[inputIdx].m_Accessor != nullptr);
				ET_ASSERT(mesh.m_ResolvedInputs[inputIdx].m_Source != nullptr);

				dae::Input const& input = mesh.m_ResolvedInputs[inputIdx].m_Input;
				dae::Accessor const& accessor = *mesh.m_ResolvedInputs[inputIdx].m_Accessor;
				dae::Source& source = *mesh.m_ResolvedInputs[inputIdx].m_Source;

				// validate input sets
				if (usedSet == dae::Input::s_InvalidIndex)
				{
					usedSet = input.m_Set;
				}
				else
				{
					if ((input.m_Set != dae::Input::s_InvalidIndex) && (input.m_Set != usedSet))
					{
						LOG(FS("COLLADA using multiple input sets: [" ET_FMT_SIZET "], [" ET_FMT_SIZET "]", usedSet, input.m_Set), core::Warning);
					}
				}

				// validate data types
				if (source.m_Type != dae::Source::E_Type::Float)
				{
					LOG("COLLADA geometry source was not made of floats, can't access data", core::Warning);
					containers.pop_back();
					return;
				}

				for (dae::Accessor::Param const& param : accessor.m_Parameters)
				{
					if (param.m_Type != dae::Accessor::E_ParamType::Float)
					{
						LOG("COLLADA geometry accessor parameters where not made of floats, can't access data", core::Warning);
						containers.pop_back();
						return;
					}
				}

				// ensure the source data is parsed
				if (!source.m_IsResolved)
				{
					if (!ColladaParser::ResolveSource(source))
					{
						LOG(FS("Failed to resolve COLLADA source '%s'r", source.m_Id.ToStringDbg()), core::Warning);
						containers.pop_back();
						return;
					}
				}

				// access data
				auto const getVecFn = [&meshContainer, &input]() -> std::vector<vec3>&
				{
					switch (input.m_Semantic)
					{
					case dae::E_Semantic::Position: return meshContainer.m_Positions;
					case dae::E_Semantic::Normal: return meshContainer.m_Normals;
					case dae::E_Semantic::Binormal: return meshContainer.m_BiNormals;
					case dae::E_Semantic::Tangent: return meshContainer.m_Tangents;
		
					default:
						ET_ASSERT(false, "Unhandled input semantic");
						return *reinterpret_cast<std::vector<vec3>*>(nullptr);
					}
				};

				switch (input.m_Semantic)
				{
				case dae::E_Semantic::Position:
				{
					// coordinate conversion
					ivec3 axisIndices = asset.Get3DIndices();
					vec3 multiplier = asset.Get3DAxisMultipliers() * asset.m_UnitToMeter;

					std::vector<vec3>& vec = getVecFn();
					for (size_t idx = input.m_Offset; idx < mesh.m_PrimitiveIndices.size(); idx += increment)
					{
						size_t const accessorIdx = mesh.m_PrimitiveIndices[idx];
						if (accessorIdx >= accessor.m_Count)
						{
							LOG("COLLADA failed to read vector from accessor, index out of bounds", core::Warning);
							containers.pop_back();
							return;
						}

						vec3 inVec = accessor.ReadVector<3>(source, accessorIdx);
						if (useTransform)
						{
							inVec = (meshTransform * vec4(inVec, 1.f)).xyz;
						}

						vec.push_back(math::swizzle(inVec * multiplier, axisIndices));
					}
				}
				break;

				case dae::E_Semantic::Normal:
				case dae::E_Semantic::Binormal:
				case dae::E_Semantic::Tangent:
				{
					// coordinate conversion
					ivec3 axisIndices = asset.Get3DIndices();
					vec3 multiplier = asset.Get3DAxisMultipliers();

					std::vector<vec3>& vec = getVecFn();
					for (size_t idx = input.m_Offset; idx < mesh.m_PrimitiveIndices.size(); idx += increment)
					{
						size_t const accessorIdx = mesh.m_PrimitiveIndices[idx];
						if (accessorIdx >= accessor.m_Count)
						{
							LOG("COLLADA failed to read vector from accessor, index out of bounds", core::Warning);
							containers.pop_back();
							return;
						}

						vec3 inVec = accessor.ReadVector<3>(source, accessorIdx);
						if (useTransform)
						{
							inVec = stationary * inVec;
						}

						vec.push_back(math::swizzle(inVec * multiplier, axisIndices));
					}
				}
				break;

				case dae::E_Semantic::Color:
				{
					std::vector<vec4>& vec = meshContainer.m_Colors;
					for (size_t idx = input.m_Offset; idx < mesh.m_PrimitiveIndices.size(); idx += increment)
					{
						size_t const accessorIdx = mesh.m_PrimitiveIndices[idx];
						if (accessorIdx >= accessor.m_Count)
						{
							LOG("COLLADA failed to read vector from accessor, index out of bounds", core::Warning);
							containers.pop_back();
							return;
						}

						vec.push_back(accessor.ReadVector<4>(source, accessorIdx));
					}
				}
				break;

				case dae::E_Semantic::Texcoord:
				{
					std::vector<vec2>& vec = meshContainer.m_TexCoords;
					for (size_t idx = input.m_Offset; idx < mesh.m_PrimitiveIndices.size(); idx += increment)
					{
						size_t const accessorIdx = mesh.m_PrimitiveIndices[idx];
						if (accessorIdx >= accessor.m_Count)
						{
							LOG("COLLADA failed to read vector from accessor, index out of bounds", core::Warning);
							containers.pop_back();
							return;
						}

						vec2 tc = accessor.ReadVector<2>(source, accessorIdx);
						tc.y = 1.f - tc.y; // we need to flip texcoords for rendering in this engine
						vec.push_back(tc);
					}
				}
				break;

				default:
					ET_ASSERT(false, "Unhandled input semantic");
				}
			}

			// other container data
			//----------------------
			if (!nodeName.empty())
			{
				meshContainer.m_Name = nodeName;
			}
			else
			{
				meshContainer.m_Name = ColladaParser::GetLibraryElementName(geometryEl);
			}

			meshContainer.m_VertexCount = meshContainer.m_Positions.size();

			// generate index buffer - this should later be optimized by removing duplicates
			meshContainer.m_Indices.reserve(meshContainer.m_VertexCount);
			for (size_t idx = 0; idx < meshContainer.m_VertexCount; ++idx)
			{
				meshContainer.m_Indices.push_back(idx);
			}

			if (m_RemoveDuplicateVertices)
			{
				meshContainer.RemoveDuplicateVertices();
			}

			if (!mesh.m_VertexCounts.empty())
			{
				if (!meshContainer.Triangulate(mesh.m_VertexCounts))
				{
					LOG("Failed to triangulate collada mesh, skipping", core::Warning);
					containers.pop_back();
					return;
				}
			}

			// derived tangent space
			if (m_CalculateTangentSpace && (meshContainer.m_Normals.size() == meshContainer.m_VertexCount))
			{
				if (meshContainer.m_Tangents.empty() && meshContainer.m_BiNormals.empty())
				{
					std::vector<vec4> tangentVec;
					meshContainer.ConstructTangentSpace(tangentVec);
				}
				else if (meshContainer.m_Tangents.empty() && (meshContainer.m_BiNormals.size() == meshContainer.m_VertexCount))
				{
					meshContainer.m_Tangents.reserve(meshContainer.m_VertexCount);
					for (size_t idx = 0u; idx < meshContainer.m_VertexCount; ++idx)
					{
						meshContainer.m_Tangents.push_back(-math::cross(meshContainer.m_Normals[idx], meshContainer.m_BiNormals[idx]));
					}
				}
				else if (meshContainer.m_BiNormals.empty() && (meshContainer.m_Tangents.size() == meshContainer.m_VertexCount))
				{
					meshContainer.m_BiNormals.reserve(meshContainer.m_VertexCount);
					for (size_t idx = 0u; idx < meshContainer.m_VertexCount; ++idx)
					{
						meshContainer.m_BiNormals.push_back(math::cross(meshContainer.m_Normals[idx], meshContainer.m_Tangents[idx]));
					}
				}
			}
		});


		// convert mesh containers to mesh assets
		//----------------------------------------

		for (pl::MeshDataContainer const& meshContainer : containers)
		{
			pl::EditableMeshAsset* const editableMeshAsset = new pl::EditableMeshAsset();
			outAssets.push_back(editableMeshAsset);

			render::MeshAsset* const meshAsset = new render::MeshAsset();
			editableMeshAsset->SetAsset(meshAsset);

			pl::EditableMeshAsset::WriteToEtMesh(&meshContainer, meshAsset->GetLoadData());
			if (containers.size() == 1u)
			{
				meshAsset->SetName(core::FileUtil::RemoveExtension(core::FileUtil::ExtractName(filePath)) + "." + pl::EditableMeshAsset::s_EtMeshExt);
			}
			else
			{
				meshAsset->SetName(meshContainer.m_Name + "." + pl::EditableMeshAsset::s_EtMeshExt);
			}
		}

		containers.clear();
	}

	return true;
}


} // namespace edit
} // namespace et

