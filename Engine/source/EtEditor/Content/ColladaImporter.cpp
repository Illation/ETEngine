#include "stdafx.h"
#include "ColladaImporter.h"

#include <gtkmm/label.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/separator.h>
#include <gtkmm/box.h>

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
	makeOptionFn("Remove duplicate vertices", m_RemoveDuplicateVertices, false);
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
		parser.IterateGeometries([](core::XML::Element const& geometryEl, dae::Asset const& asset)
		{
			// ensure we have all the relevant XML elements
			//----------------------------------------------
			core::XML::Element const* const meshEl = geometryEl.GetFirstChild("mesh"_hash);
			if (meshEl == nullptr)
			{
				return; // for now we only support mesh geometries
			}

			core::XML::Element const* const verticesEl = meshEl->GetFirstChild("vertices"_hash);
			if (verticesEl == nullptr)
			{
				LOG("Expected COLLADA mesh to have vertices element!", core::Warning);
				return;
			}

			core::XML::Attribute const* const vertexIdAttrib = verticesEl->GetAttribute("id"_hash);
			if (vertexIdAttrib == nullptr)
			{
				LOG("Expected COLLADA vertices to have id attribute!", core::Warning);
				return;
			}

			core::XML::Element const* primitive = meshEl->GetFirstChild("triangles"_hash);
			if (primitive == nullptr)
			{
				primitive = meshEl->GetFirstChild("polylist"_hash);
				if (primitive == nullptr)
				{
					return; // for now we only support triangle or polylist meshes
				}
			}

			if (ColladaParser::GetPrimitiveCount(*meshEl) > 1u)
			{
				LOG("COLLADA mesh had more than one primitive, ignoring subsequent occurances!", core::Warning);
			}

			core::XML::Element const* const primitiveArrayEl = primitive->GetFirstChild("p"_hash);
			if (primitiveArrayEl == nullptr)
			{
				LOG("Expected COLLADA primitive to have a 'p' element!", core::Warning);
				return;
			}

			// setup dataflow
			//----------------
	
			std::vector<dae::Source> sources;
			ColladaParser::ReadSourceList(sources, *meshEl);

			struct ResolvedInput
			{
				dae::Input m_Input;
				dae::Accessor const* m_Accessor;
				dae::Source const* m_Source;
			};

			std::vector<ResolvedInput> resolvedInputs;
			size_t maxInputOffset = 0u;

			{
				auto resolveInputFn = [&](ResolvedInput& input) -> bool
				{
					// accessor from input
					auto const foundAccessorSourceIt = std::find_if(sources.cbegin(), sources.cend(), [&input](dae::Source const& source)
						{
							return (source.m_Id == input.m_Input.m_Source);
						});

					if ((foundAccessorSourceIt == sources.cend()) || (foundAccessorSourceIt->m_CommonAccessor == nullptr))
					{
						LOG(FS("Failed to find accessor '%s' for input", input.m_Input.m_Source.ToStringDbg()), core::Warning);
						return false;
					}

					input.m_Accessor = foundAccessorSourceIt->m_CommonAccessor;

					// source from accessor
					auto const foundSourceIt = std::find_if(sources.begin(), sources.end(), [&input](dae::Source const& source)
						{
							return (source.m_DataId == input.m_Accessor->m_SourceDataId);
						});

					if (foundSourceIt == sources.cend())
					{
						LOG(FS("Failed to find source '%s' for accessor", input.m_Accessor->m_SourceDataId.ToStringDbg()), core::Warning);
						return false;
					}

					input.m_Source = &(*foundSourceIt);
					// parse source;

					if (input.m_Input.m_Offset > maxInputOffset)
					{
						maxInputOffset = input.m_Input.m_Offset;
					}

					resolvedInputs.push_back(input);
					return true;
				};

				std::vector<dae::Input> vertexInputs;
				ColladaParser::ReadInputList(vertexInputs, *verticesEl, false);

				std::vector<dae::Input> primitiveInputs;
				ColladaParser::ReadInputList(primitiveInputs, *primitive, true);

				core::HashString const verticesId(vertexIdAttrib->m_Value.c_str());
				for (dae::Input const& input : primitiveInputs)
				{
					ET_ASSERT(input.m_Semantic != dae::E_Semantic::Invalid);

					if (input.m_Semantic == dae::E_Semantic::Vertex)
					{
						if (input.m_Source != verticesId)
						{
							LOG("Expected VERTEX input to use vertices as source", core::Warning);
							return;
						}

						for (dae::Input const& vertexInput : vertexInputs)
						{
							ResolvedInput resolvedInput;
							resolvedInput.m_Input = vertexInput;
							resolvedInput.m_Input.m_Offset = input.m_Offset;
							resolvedInput.m_Input.m_Set = input.m_Set;

							if (!resolveInputFn(resolvedInput))
							{
								return;
							}
						}
					}
					else
					{
						ResolvedInput resolvedInput;
						resolvedInput.m_Input = input;

						if (!resolveInputFn(resolvedInput))
						{
							return;
						}
					}
				}
			}

			// read sources
			//--------------
			std::vector<size_t> indices;
			ColladaParser::ParseArray(indices, *primitiveArrayEl);
		});
	}

	return true;
}


} // namespace edit
} // namespace et

