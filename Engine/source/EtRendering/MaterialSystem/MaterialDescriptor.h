#pragma once
#include <EtCore/Reflection/Registration.h>
#include <EtCore/Content/AssetPointer.h>

#include <EtRHI/GraphicsTypes/ParameterBlock.h>


namespace et {
	REGISTRATION_NS(render)
}


namespace et {
namespace render {


class rhi::ShaderData;
class rhi::TextureData;


//---------------------------------
// BaseMaterialParam
//
// Abstract material parameter
//
class BaseMaterialParam
{
	// definitions
	//-------------
	RTTR_ENABLE()
	REGISTRATION_FRIEND_NS(render)

	// construct destruct
	//---------------------
public:
	virtual ~BaseMaterialParam() = default;

	// interface
	//-----------
	virtual rttr::type GetType() const = 0;

	// funtionality
	//--------------

	// accessors
	//-----------
	core::HashString const GetId() const { return m_Id; }

	// Data
	///////

private:
	core::HashString m_Id;
};

//---------------------------------
// MaterialParam
//
// Templated material parameter including the actual data
//
template <typename TDataType>
class MaterialParam final : public BaseMaterialParam
{
	// definitions
	//-------------
	RTTR_ENABLE(BaseMaterialParam)
	REGISTRATION_FRIEND_NS(render)

	// construct destruct
	//--------------------
public:
	MaterialParam() : BaseMaterialParam() {}
	~MaterialParam() = default;

	// material param interface
	//--------------------------
	rttr::type GetType() const override { return rttr::type::get<TDataType>(); }

	// accessors
	//-----------
	TDataType const& GetData() const { return m_Data; }

	// Data
	///////
private:
	TDataType m_Data;
};

//---------------------------------
// MaterialDescriptor
//
// Serializable material info
//
struct MaterialDescriptor
{
	RTTR_ENABLE()
public:

	std::vector<BaseMaterialParam*> parameters;
};


//---------------------------------
// parameters
//
// utility functionality to deal with parameter data
//
namespace parameters {


void ConvertDescriptor(rhi::T_ParameterBlock const baseParams,
	MaterialDescriptor const& desc, 
	rhi::ShaderData const* const shader, 
	std::vector<AssetPtr<rhi::TextureData>> const& textureRefs);


} // namespace parameters


} // namespace render
} // namespace et
