#pragma once
#include <RmlUi/Core/Input.h>
#include <RmlUi/Core/Types.h>
#include <RmlUi/Core/DataVariable.h>
#include <RmlUi/Core/DataModelHandle.h>

#include <EtCore/Input/KeyCodes.h>
#include <EtCore/Containers/slot_map.h>


namespace et {
namespace gui {

namespace RmlUtil {

	int32 GetRmlButtonIndex(E_MouseButton const button);
	Rml::Input::KeyIdentifier GetRmlKeyId(E_KbdKey const key);
	int32 GetRmlModifierFlags(core::T_KeyModifierFlags const mods);


	// Converting et::math types to Rml counterparts
	template <typename TDataType>
	Rml::Vector2<TDataType> ToRml(math::vector<2, TDataType> const& rhs)
	{
		return Rml::Vector2<TDataType>(rhs.x, rhs.y);
	}

	template <typename TDataType>
	Rml::Vector3<TDataType> ToRml(math::vector<3, TDataType> const& rhs)
	{
		return Rml::Vector3<TDataType>(rhs.x, rhs.y, rhs.z);
	}

	template <typename TDataType>
	Rml::Vector4<TDataType> ToRml(math::vector<4, TDataType> const& rhs)
	{
		return Rml::Vector4<TDataType>(rhs.x, rhs.y, rhs.z, rhs.w);
	}

	// Converting et::math types from Rml counterparts
	template <typename TDataType>
	math::vector<2, TDataType> ToEtm(Rml::Vector2<TDataType> const& rhs)
	{
		return math::vector<2, TDataType>(rhs.x, rhs.y);
	}

	template <typename TDataType>
	math::vector<3, TDataType> ToEtm(Rml::Vector3<TDataType> const& rhs)
	{
		return math::vector<3, TDataType>(rhs.x, rhs.y, rhs.z);
	}

	template <typename TDataType>
	math::vector<4, TDataType> ToEtm(Rml::Vector4<TDataType> const& rhs)
	{
		return math::vector<4, TDataType>(rhs.x, rhs.y, rhs.z, rhs.w);
	}


	// Custom Data Definitions
	
	//-------------------
	// SlotMapDefinition
	//
	// Allow using slot maps as data model containers
	//
	template<typename TContainer>
	class SlotMapDefinition final : public Rml::VariableDefinition {
	public:
		SlotMapDefinition(Rml::VariableDefinition* underlyingDef)
			: Rml::VariableDefinition(Rml::DataVariableType::Array)
			, m_UnderlyingDefinition(underlyingDef)
		{}

		int Size(void* ptr) override 
		{
			return int(static_cast<TContainer*>(ptr)->size());
		}

	protected:
		Rml::DataVariable Child(void* voidPtr, const Rml::DataAddressEntry& address) override
		{
			TContainer* ptr = static_cast<TContainer*>(voidPtr);
			const int index = address.index;

			const int mapSize = int(ptr->size());
			if (index < 0 || !ptr->is_valid(index))
			{
				if (address.name == "size")
				{
					return Rml::MakeLiteralIntVariable(mapSize);
				}

				ET_WARNING("Data slot map index out of bounds.");
				return Rml::DataVariable();
			}

			void* dataPtr = &(*ptr->get_iterator(index));
			return Rml::DataVariable(m_UnderlyingDefinition, dataPtr);
		}

	private:
		Rml::VariableDefinition* m_UnderlyingDefinition;
	};

	template<typename TContainer>
	bool RegisterSlotMapToDataModelConstructor(Rml::DataModelConstructor& modelConstructor)
	{
		using value_type = typename TContainer::value_type;
		Rml::VariableDefinition* valueVar = modelConstructor.GetDataTypeRegister()->GetDefinition<value_type>();
		if (valueVar == nullptr)
		{
			ET_WARNING("slot map type hasn't been registered!");
			return false;
		}

		if (!modelConstructor.RegisterCustomDataVariableDefinition<TContainer>(std::move(Rml::MakeUnique<SlotMapDefinition<TContainer>>(valueVar))))
		{
			ET_WARNING("Failed to register slot map type.");
			return false;
		}

		return true;
	}


} // namespace RmlUtil

} // namespace gui
} // namespace et

