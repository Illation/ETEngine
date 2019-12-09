#include "stdafx.h"
#include "MaterialDescriptor.h"

#include "Shader.h"

#include <rttr/registration>


// reflection
///////////////////////

// definition for a parameter deriving from BaseMaterialParam -> registers the type and a convesion function to enable serialized polymorphism
#define REGISTER_DERIVED_PARAM(TYPE, NAME)														\
registration::class_<TYPE>(NAME " param")														\
	.constructor<>()(rttr::detail::as_object())													\
	.property("data", &TYPE::m_Data);															\
																								\
rttr::type::register_converter_func([](TYPE& param, bool& ok) -> render::BaseMaterialParam*		\
{																								\
	ok = true;																					\
	return new TYPE(param);																		\
}); 


// register all the types
RTTR_REGISTRATION
{
	using namespace rttr;

	// base
	registration::class_<render::BaseMaterialParam>("base material param")
		.property("name", &render::BaseMaterialParam::GetName, &render::BaseMaterialParam::SetName);

	// derived
	REGISTER_DERIVED_PARAM(render::MaterialParam<std::string>, "texture"); // convert string to texture asset reference
	REGISTER_DERIVED_PARAM(render::MaterialParam<mat4>, "matrix4x4");
	REGISTER_DERIVED_PARAM(render::MaterialParam<mat3>, "matrix3x3");
	REGISTER_DERIVED_PARAM(render::MaterialParam<vec4>, "vector4D");
	REGISTER_DERIVED_PARAM(render::MaterialParam<vec3>, "vector3D");
	REGISTER_DERIVED_PARAM(render::MaterialParam<vec2>, "vector2D");
	REGISTER_DERIVED_PARAM(render::MaterialParam<uint32>, "uint");
	REGISTER_DERIVED_PARAM(render::MaterialParam<int32>, "int");
	REGISTER_DERIVED_PARAM(render::MaterialParam<float>, "float");
	REGISTER_DERIVED_PARAM(render::MaterialParam<bool>, "boolean");

	// container
	registration::class_<render::MaterialDescriptor>("material descriptor")
		.property("parameters", &render::MaterialDescriptor::parameters);
}


namespace render {


//=========================
// Base Material Parameter
//=========================


//---------------------------------
// BaseMaterialParam::SetName
//
void BaseMaterialParam::SetName(std::string const& val)
{
	m_Name = val;
	m_Id = GetHash(m_Name);
}


namespace parameters {


//=========================
// parameters
//=========================


//---------------------------------
// ConvertDescriptor
//
// Convert a material descriptor into a shader parameter set
//
T_ParameterBlock ConvertDescriptor(MaterialDescriptor const& desc, ShaderData const* const shader, std::vector<AssetPtr<TextureData>> const& textureRefs)
{
	ET_ASSERT(shader != nullptr);

	// ideally this should be done before any uniforms are set in the shader
	T_ParameterBlock const block = shader->CopyCurrentUniforms();
	if (block == nullptr)
	{
		return block; // if the material has no parameters we just return null
	}

	std::vector<UniformParam> const& layout = shader->GetUniformLayout();
	std::vector<T_Hash> const& ids = shader->GetUniformIds();
	ET_ASSERT(layout.size() == ids.size());

	// override all 
	for (size_t paramIdx = 0u; paramIdx < layout.size(); ++paramIdx)
	{
		UniformParam const& param = layout[paramIdx];
		T_Hash const paramId = ids[paramIdx];

		// try finding the corresponding parameter in the descriptor
		auto paramIt = std::find_if(desc.parameters.cbegin(), desc.parameters.cend(), [paramId](BaseMaterialParam const* const baseParam)
			{
				return baseParam->GetId() == paramId;
			});

		// if we don't find it that's fine
		if (paramIt == desc.parameters.cend())
		{
			continue;
		}

		BaseMaterialParam const* const baseParam = *paramIt;
		switch (param.type)
		{
		case E_ParamType::Texture2D:
		case E_ParamType::Texture3D:
		case E_ParamType::TextureCube:
		case E_ParamType::TextureShadow:
			{
				ET_ASSERT(baseParam->GetType() == typeid(std::string));
				T_Hash const assetId = GetHash(static_cast<MaterialParam<std::string> const*>(baseParam)->GetData());

				auto texIt = std::find_if(textureRefs.cbegin(), textureRefs.cend(), [assetId](AssetPtr<TextureData> const& texture)
					{
						return texture.GetAsset()->GetId() == assetId;
					});

				if (texIt == textureRefs.cend())
				{
					ET_ASSERT(false, "Unreferenced texture parameter!");
					break;
				}

				render::parameters::Write(block, param.offset, (*texIt).get());
			}
			break;

		case E_ParamType::Matrix4x4:
			ET_ASSERT(baseParam->GetType() == typeid(mat4));
			parameters::Write(block, param.offset, static_cast<MaterialParam<mat4> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Matrix3x3:
			ET_ASSERT(baseParam->GetType() == typeid(mat3));
			parameters::Write(block, param.offset, static_cast<MaterialParam<mat3> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Vector4:
			ET_ASSERT(baseParam->GetType() == typeid(vec4));
			parameters::Write(block, param.offset, static_cast<MaterialParam<vec4> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Vector3:
			ET_ASSERT(baseParam->GetType() == typeid(vec3));
			parameters::Write(block, param.offset, static_cast<MaterialParam<vec3> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Vector2:
			ET_ASSERT(baseParam->GetType() == typeid(vec2));
			parameters::Write(block, param.offset, static_cast<MaterialParam<vec2> const*>(baseParam)->GetData());
			break;

		case E_ParamType::UInt:
			ET_ASSERT(baseParam->GetType() == typeid(uint32));
			parameters::Write(block, param.offset, static_cast<MaterialParam<uint32> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Int:
			ET_ASSERT(baseParam->GetType() == typeid(int32));
			parameters::Write(block, param.offset, static_cast<MaterialParam<int32> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Float:
			ET_ASSERT(baseParam->GetType() == typeid(float));
			parameters::Write(block, param.offset, static_cast<MaterialParam<float> const*>(baseParam)->GetData());
			break;

		case E_ParamType::Boolean:
			ET_ASSERT(baseParam->GetType() == typeid(bool));
			parameters::Write(block, param.offset, static_cast<MaterialParam<bool> const*>(baseParam)->GetData());
			break;

		default:
			ET_ASSERT(false, "Unhandled parameter type");
			break;
		}
	}

	return block;
}


} // namespace parameters
} // namespace render
