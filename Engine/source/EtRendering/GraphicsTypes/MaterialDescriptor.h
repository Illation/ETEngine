#pragma once
#include "ParameterBlock.h"

#include <rttr/registration_friend.h>

#include <EtCore/Content/AssetPointer.h>


class ShaderData;
class TextureData;


namespace render {


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

	// construct destruct
	//---------------------
public:
	virtual ~BaseMaterialParam() = default;

	// interface
	//-----------
	virtual std::type_info const& GetType() const = 0;

	// funtionality
	//--------------
	void SetName(std::string const& val);

	// accessors
	//-----------
	std::string const& GetName() const { return m_Name; }
	T_Hash const GetId() const { return m_Id; }

	// Data
	///////
private:

	std::string m_Name;
	T_Hash m_Id = 0u;
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
	RTTR_REGISTRATION_FRIEND

	// construct destruct
	//--------------------
public:
	MaterialParam() : BaseMaterialParam() {}
	~MaterialParam() = default;

	// material param interface
	//--------------------------
	std::type_info const& GetType() const override { return typeid(TDataType); }

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


T_ParameterBlock ConvertDescriptor(MaterialDescriptor const& desc, ShaderData const* const shader, std::vector<AssetPtr<TextureData>> const& textureRefs);


} // namespace parameters
} // namespace render
