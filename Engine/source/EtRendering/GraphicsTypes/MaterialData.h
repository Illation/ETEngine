#pragma once

namespace render {


class I_MaterialParam
{
public:
	virtual ~I_MaterialParam = default;

	virtual std::type_info const& GetType() const = 0;
	virtual std::string const& GetName() const = 0;
	virtual T_Hash const GetId() const = 0;
};

template <typename TDataType>
class MaterialParam final : public I_MaterialParam;
{
public:
	MaterialParam(std::string const& name = std::string()) : I_MaterialParam(), m_Name(name), m_Id(GetHash(name)) {}
	~MaterialParam() = default;

	std::type_info const& GetType() const override { return typeid(TDataType); }
	std::string const& GetName() const override { return m_Name; }
	T_Hash const GetId() const override { return m_Id; }

	void Set(TDataType const& value) { m_Data = value; }

private:
	std::string m_Name;
	T_Hash m_Id = 0u;
	TDataType m_Data;
};

class MaterialInstance;

class Material
{
public:
	enum class E_DrawType
	{
		Opaque,
		Translucent
	};

private:
	E_DrawType m_DrawType = E_DrawType::Opaque;
	AssetPtr<ShaderData> m_Shader;
	MaterialInstance m_BaseInstance;

	T_VertexFlags m_LayoutFlags = 0u;
	std::vector<int32> m_AttributeLocations;
	std::vector<int32> m_ParamLocations;
};

class MaterialInstance final
{
	template <typename TDataType>
	TDataType const& GetParameter(T_Hash const id) const;

	template <typename TDataType>
	void SetParameter(T_Hash const id, TDataType const& value);

private:
	AssetPtr<MaterialInstance> m_Parent;
	AssetPtr<Material> m_Material;
	std::vector<I_MaterialParam*> m_Parameters;
};


} // namespace render

