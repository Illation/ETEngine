#include "stdafx.h"
#include "JsonDeserializer.h"

#include <EtCore/IO/JsonDom.h>
#include <EtCore/IO/JsonWriter.h>
#include <EtCore/Hashing/HashString.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetRegistration.h>

#include "registerMath.h"


namespace et {
namespace core {


//===================
// JSON Deserializer
//===================


//-------------------------------------
// JsonDeserializer::ExtractBasicTypes
//
// Convert basic JSON "leaf" values to rttr::variants, assuming the variant will convert it to its actual type later
// If the JSON value is not a basic type we return an invalid variant
//
rttr::variant JsonDeserializer::ExtractBasicTypes(JSON::Value const* const jVal)
{
	switch (jVal->GetType())
	{
	case JSON::JSON_String:
		return jVal->str()->value;
	case JSON::JSON_Bool:
		return jVal->b()->value;
	case JSON::JSON_Number:
	{
		if (jVal->num()->isInt)
		{
			return jVal->num()->valueInt;
		}
		else
		{
			return jVal->num()->value;
		}
	}

	// we handle only the basic types here
	case JSON::JSON_Null:
	case JSON::JSON_Object:
	case JSON::JSON_Array:
		break;
	}

	return rttr::variant(); // invalid
}

//------------------------------------------
// JsonDeserializer::ArrayFromJsonRecursive
//
// deserialize a JSON array into an rttr sequential view
//  - returns false if any of the elements failes deserialization, but tries to parse the entire view anyway
//
bool JsonDeserializer::ArrayFromJsonRecursive(rttr::variant_sequential_view& view, JSON::Value const* const jVal)
{
	if (jVal->GetType() != JSON::JSON_Array)
	{
		LOG("ArrayFromJsonRecursive > Expected JSON::Value to be of type array!", LogLevel::Warning);
		return false;
	}
	JSON::Array const* const jArr = jVal->arr();

	if (view.get_size() != jArr->value.size())
	{
		view.set_size(jArr->value.size());
	}
	rttr::type const arrayValueType = view.get_rank_type(1);


	bool success = true;

	// Loop over array elements and fill them, depending on the internal type
	for (size_t i = 0; i < jArr->value.size(); ++i)
	{
		JSON::Value const* jIndexVal = jArr->value[i];
		rttr::type localType = arrayValueType;

		// pointers should be wrapped
		bool isNull;
		if (!ExtractPointerValueType(localType, jIndexVal, isNull))
		{
			success = false;
			continue;
		}

		if (isNull)
		{
			view.get_value(i) = nullptr;
			continue;
		}

		if (jIndexVal->GetType() == JSON::JSON_Array) // multi dimensional array
		{
			if (IsVectorType(localType))
			{
				rttr::array_range<rttr::property> const prop_list = localType.get_properties();
				ET_ASSERT(prop_list.size() == 1);

				auto prop = *prop_list.begin();
				ET_ASSERT(prop.get_name().to_string() == "data");

				rttr::instance inst(view.get_value(i));
				rttr::type propValType = prop.get_type();
				rttr::variant propVar = prop.get_value(inst);

				ET_ASSERT(propValType.is_sequential_container());

				auto subArrayView = propVar.create_sequential_view();

				if ((!ArrayFromJsonRecursive(subArrayView, jIndexVal)) || !propVar.is_valid())
				{
					LOG("ArrayFromJsonRecursive > There was an issue deserializing the vectors / matrices sequential view, typeName: '"
						+ propValType.get_name().to_string() + std::string("'!"), LogLevel::Warning);
				}
				else
				{
					prop.set_value(inst, propVar);
				}
			}
			else
			{
				auto subArrayView = view.get_value(i).create_sequential_view();
				if (!ArrayFromJsonRecursive(subArrayView, jIndexVal))
				{
					LOG("ArrayFromJsonRecursive > There was an issue deserializing the inner array, index: #" + std::to_string(i)
						+ std::string(" typeName: '") + localType.get_name().to_string() + std::string("'!"), LogLevel::Warning);

					success = false;
				}
			}
		}
		else if (jIndexVal->GetType() == JSON::JSON_Object) // array of objects
		{
			rttr::variant tempVar = view.get_value(i);
			rttr::variant wrappedVar = tempVar.extract_wrapped_value();

			// for pointers we will have to create the type
			if (localType != arrayValueType)
			{
				// find the right constructor for our type
				rttr::constructor ctor = localType.get_constructor();

				//use it
				if (ctor.is_valid())
				{
					wrappedVar = ctor.invoke();
				}
				else
				{
					LOG("ArrayFromJsonRecursive > Failed to get a valid constructor from property, index: #" + std::to_string(i)
						+ std::string(" typeName: '") + localType.get_name().to_string() + std::string("'!"), LogLevel::Warning);

					success = false;
					continue;
				}
			}

			ObjectFromJsonRecursive(jIndexVal, wrappedVar, localType);

			if (!wrappedVar.is_valid())
			{
				LOG("ArrayFromJsonRecursive > Failed to create a valid object from property, index: #" + std::to_string(i)
					+ std::string(" typeName: '") + localType.get_name().to_string() + std::string("'!"), LogLevel::Warning);

				success = false;
			}

			if (localType != arrayValueType)
			{
				wrappedVar.convert(arrayValueType);
			}

			view.set_value(i, wrappedVar);
		}
		else // array of basic types
		{
			rttr::type const& localTypeCRef = localType;
			rttr::variant extractedVal = ExtractBasicTypes(jIndexVal);
			if (extractedVal.convert(localTypeCRef))
			{
				view.set_value(i, extractedVal);
			}
			else
			{
				LOG("ArrayFromJsonRecursive > Failed to convert basic type extracted from JSON to property value type, index: #" + std::to_string(i)
					 + std::string(" typeName: '") + localTypeCRef.get_name().to_string() + std::string("'!"), LogLevel::Warning);

				success = false;
			}
		}
	}

	return success;
}

//--------------------------------
// JsonDeserializer::ExtractValue
//
// Extracts a json basic type or object to an rttr variant, if it is neither it will return an invalid variant
//
rttr::variant JsonDeserializer::ExtractValue(JSON::Value const* const jVal, const rttr::type& valueType)
{
	// try converting from a basic type
	rttr::variant extractedVal = ExtractBasicTypes(jVal);

	// if that doesn't work, try an object
	if (!extractedVal.convert(valueType))
	{
		if ((jVal->GetType() == JSON::JSON_Object) || (jVal->GetType() == JSON::JSON_Null))
		{
			JSON::Value const* localJVal = jVal;
			rttr::type localType = valueType;
			bool isNull;
			if (!ExtractPointerValueType(localType, localJVal, isNull))
			{
				return rttr::variant();
			}

			if (isNull)
			{
				return nullptr;
			}

			// find the right constructor for our type
			rttr::constructor ctor = valueType.get_constructor();
			for (auto& item : valueType.get_constructors())
			{
				if (item.get_instantiated_type() == valueType)
				{
					ctor = item;
				}
			}

			//use it
			extractedVal = ctor.invoke();

			// fill the rest of our object
			ObjectFromJsonRecursive(localJVal, extractedVal, localType);
		}
	}

	return extractedVal;
}

//----------------------------------------------------
// JsonDeserializer::AssociativeViewFromJsonRecursive
//
// deserialize a JSON array into an rttr associative view (dictionary / map)
//  - returns false if any of the key value pairs failes deserialization, but tries to parse the entire view anyway
//
bool JsonDeserializer::AssociativeViewFromJsonRecursive(rttr::variant_associative_view& view, JSON::Value const* const jVal)
{
	if (jVal->GetType() != JSON::JSON_Array)
	{
		LOG("AssociativeViewFromJsonRecursive > Expected JSON::Value to be of type array!", LogLevel::Warning);
		return false;
	}
	JSON::Array const* const jArr = jVal->arr();

	bool success = true;

	for (size_t i = 0; i < jArr->value.size(); ++i)
	{
		JSON::Value const* const jIndexVal = jArr->value[i];

		if (jIndexVal->GetType() == JSON::JSON_Object) // a key-value associative view
		{
			// get the json object
			JSON::Object const* const jIndexObj = jIndexVal->obj();

			// extract the key
			auto jKeyIt = std::find_if(jIndexObj->value.begin(), jIndexObj->value.end(), [](JSON::Pair const& el)
			{
				return el.first == std::string("key");
			});

			if (jKeyIt == jIndexObj->value.cend())
			{
				LOG("AssociativeViewFromJsonRecursive > Failed to find the value of 'key' in json object, index #" + std::to_string(i)
					, LogLevel::Warning);

				success = false;
				continue;
			}

			rttr::variant key_var = ExtractValue(jKeyIt->second, view.get_key_type());
			if (!key_var.is_valid())
			{
				LOG("AssociativeViewFromJsonRecursive > Failed to create a valid variant from key, index #" + std::to_string(i) +
					std::string(" typeName: '") + view.get_key_type().get_name().to_string() + std::string("'!"), LogLevel::Warning);

				success = false;
				continue;
			}

			// extract the value
			auto jValIt = std::find_if(jIndexObj->value.begin(), jIndexObj->value.end(), [](JSON::Pair const& el)
			{
				return el.first == std::string("value");
			});

			if (jValIt == jIndexObj->value.cend())
			{
				LOG("AssociativeViewFromJsonRecursive > Failed to find the value of 'value' in json object, index #" + std::to_string(i)
					, LogLevel::Warning);

				success = false;
				continue;
			}

			rttr::variant value_var = ExtractValue(jValIt->second, view.get_value_type());
			if (!value_var.is_valid())
			{
				LOG("AssociativeViewFromJsonRecursive > Failed to create a valid object from value, index #" + std::to_string(i) +
					std::string(" typeName: '") + view.get_value_type().get_name().to_string() + std::string("'!"), LogLevel::Warning);

				success = false;
				continue;
			}

			// insert the key value pair into associate container
			view.insert(key_var, value_var);
		}
		else // a key-only associative view
		{
			rttr::variant extractedVal = ExtractBasicTypes(jIndexVal);
			if (extractedVal && extractedVal.convert(view.get_key_type()))
			{
				view.insert(extractedVal);
			}
			else
			{
				LOG("AssociativeViewFromJsonRecursive > Failed to convert basic type extracted from JSON to property value type, index: #" 
					+ std::to_string(i) + std::string(" typeName: '") + view.get_key_type().get_name().to_string() + std::string("'!")
					, LogLevel::Warning);

				success = false;
			}
		}
	}

	return success;
}

//---------------------------------
// JsonDeserializer::FromJsonValue
//
// Extract a JSON Value into a variant using its value type
//
void JsonDeserializer::FromJsonValue(JSON::Value const* jVal, rttr::type &valueType, rttr::variant &var)
{
	rttr::type localType = valueType;
	bool isNull;
	if (!ExtractPointerValueType(localType, jVal, isNull))
	{
		return;
	}

	if (isNull)
	{
		var = nullptr; 
		return;
	}

	switch (jVal->GetType())
	{
		case JSON::JSON_Array:
		{
			if (localType.is_sequential_container())
			{
				auto view = var.create_sequential_view();

				if (!ArrayFromJsonRecursive(view, jVal))
				{
					LOG("FromJsonValue > There was an issue deserializing the sequential view, typeName: '"
						+ localType.get_name().to_string() + std::string("'!"), LogLevel::Warning);
				}
			}
			else if (localType.is_associative_container())
			{
				auto associativeView = var.create_associative_view();

				if (!AssociativeViewFromJsonRecursive(associativeView, jVal))
				{
					LOG("FromJsonValue > There was an issue deserializing the associate view, typeName: '"
						+ localType.get_name().to_string() + std::string("'!"), LogLevel::Warning);
				}
			}
			else if (IsVectorType(localType))
			{
				rttr::array_range<rttr::property> const prop_list = localType.get_properties();
				ET_ASSERT(prop_list.size() == 1);

				auto prop = *prop_list.begin();
				ET_ASSERT(prop.get_name().to_string() == "data");

				rttr::instance inst(var);
				rttr::type propValType = prop.get_type();
				rttr::variant propVar = prop.get_value(inst);

				ET_ASSERT(propValType.is_sequential_container());

				auto view = propVar.create_sequential_view();

				if ((!ArrayFromJsonRecursive(view, jVal)) || !propVar.is_valid())
				{
					LOG("FromJsonValue > There was an issue deserializing the vectors / matrices sequential view, typeName: '"
						+ propValType.get_name().to_string() + std::string("'!"), LogLevel::Warning);
				}
				else 
				{
					prop.set_value(inst, propVar);
				}
			}
			else
			{
				LOG("FromJsonValue > Found a JSON value of type array, but the property is not a sequential or associate container, or vector type: '"
					+ localType.get_name().to_string() + std::string("'!"), LogLevel::Warning);
			}

			break;
		}

		case JSON::JSON_Object:
		{
			// for pointers we will have to create the type
			if (localType != valueType)
			{
				// find the right constructor for our type
				rttr::constructor ctor = localType.get_constructor();

				//use it
				if (ctor.is_valid())
				{
					var = ctor.invoke();
				}
				else
				{
					LOG(FS("FromJsonValue > Failed to get a valid constructor from property, typeName: '%s'!", localType.get_name().data()), 
						LogLevel::Warning);

					break;
				}
			}

			ObjectFromJsonRecursive(jVal, var, localType);

			if (!var.is_valid())
			{
				LOG("FromJsonValue > Failed to create a valid object from property, typeName: '"
					+ localType.get_name().to_string() + std::string("'!"), LogLevel::Warning);
				break;
			}

			if (localType != valueType)
			{
				var.convert(rttr::type(valueType));
			}

			break;
		}

		default:
		{
			rttr::type const& vType = localType;

			var = ExtractBasicTypes(jVal); // extract the basic type to a variant
			if (!(var.convert(vType))) // then try to convert it to the type of our property
			{
				LOG("FromJsonValue > Failed to convert basic type extracted from JSON to property value type, typeName: '"
					+ localType.get_name().to_string() + std::string("'!"), LogLevel::Warning);
			}
		}
	}
}

//-------------------------------------------
// JsonDeserializer::ExtractPointerValueType
//
// Pointer objects are wrapped so that they can indicate their underlying inherited type
//
bool JsonDeserializer::ExtractPointerValueType(rttr::type &inOutValType, JSON::Value const* &inOutJVal, bool& isNull)
{
	isNull = false;

	if (inOutValType.is_pointer())
	{
		inOutValType = inOutValType.get_raw_type();

		if (inOutJVal->GetType() == JSON::JSON_Null)
		{
			isNull = true;
			return true;
		}
		else if (inOutJVal->GetType() != JSON::JSON_Object)
		{
			LOG("ExtractPointerValueType > Expected JSON::Value to be of type object!", LogLevel::Warning);
			return false;
		}
		JSON::Object const* const jObj = inOutJVal->obj();

		ET_ASSERT(jObj->value.size() == 1u, "Expected pointer JSON object to have exactly one internal value!");

		// figure out what kind of object we are deserializing
		std::string internalTypeName = jObj->value[0].first;

		if (inOutValType.get_name().to_string() != internalTypeName)// if it's not a pointer to the base type, check the derived types 
		{
			// get all derived types
			rttr::array_range<rttr::type> derivedTypes = inOutValType.get_derived_classes();

			// try finding our internal type in that list
			auto foundTypeIt = std::find_if(derivedTypes.begin(), derivedTypes.end(), [&internalTypeName](rttr::type const& el)
			{
				return el.get_name().to_string() == internalTypeName;
			});

			if (foundTypeIt == derivedTypes.cend())
			{
				LOG("ExtractPointerValueType > Pointers internal type doesn't derive from class type!", LogLevel::Warning);
				return false;
			}

			inOutValType = *foundTypeIt;
		}

		inOutJVal = jObj->value[0].second;
	}

	return true;
}

//-------------------------------------------
// JsonDeserializer::ObjectFromJsonRecursive
//
// Recursively deserialize JSON values into an object or pointer(the instance) with a known type
//
void JsonDeserializer::ObjectFromJsonRecursive(JSON::Value const* const jVal, rttr::instance const &inst, rttr::type &instType)
{
	if (jVal->GetType() != JSON::JSON_Object)
	{
		LOG("ObjectFromJsonRecursive > Expected JSON::Value to be of type object!", LogLevel::Warning);
		return;
	}
	JSON::Object const* const jObj = jVal->obj();

	rttr::instance instObject = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance() : inst;
	rttr::array_range<rttr::property> const prop_list = instType.get_properties();

	for (auto prop : prop_list)
	{
		std::string propName = prop.get_name().to_string();

		// find children in json, if not just skip them
		auto jChildIt = std::find_if(jObj->value.begin(), jObj->value.end(), [propName](JSON::Pair const& el)
		{
			return el.first == propName;
		});
		if (jChildIt == jObj->value.cend())
		{
			// not specifying a property is valid and not an error, the default value should be used
			continue;
		}

		rttr::type propValType = prop.get_type();
		rttr::variant var = prop.get_value(instObject);
		JSON::Value const* propJVal = jChildIt->second;

		FromJsonValue(propJVal, propValType, var);

		if (var.is_valid())
		{
			if (var.get_type() != rttr::type::get<std::nullptr_t>())
			{
				prop.set_value(instObject, var);
			}
			else
			{
				prop.set_value(instObject, nullptr);
			}
		}
		else
		{
			LOG("ObjectFromJsonRecursive > extracted variant was invalid, property: '" + propName + std::string("' typeName: '") +
				propValType.get_name().to_string() + std::string("'!"), LogLevel::Warning);
		}
	}
}

//-------------------------------------
// JsonDeserializer::FromJsonRecursive
//
// Recursively deserialize JSON values into an object or pointer(the instance)
//
void JsonDeserializer::FromJsonRecursive(rttr::instance const inst, JSON::Object const* const parentObj) // assumes jVal is a JSON::Object
{
	rttr::type instType = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance().get_derived_type() : inst.get_derived_type();

	// try finding a json value in its parent by the typename
	ET_ASSERT(parentObj->value.size() == 1u);
	ET_ASSERT(parentObj->value[0].first == instType.get_name().to_string(),
		"root json object didn't contain a field corresponding to the deserialized type '%s'",
		instType.get_name().data());

	// deserialize
	ObjectFromJsonRecursive(parentObj->value[0].second, inst, instType);

	// do necessary conversions and checks
	if (!inst.is_valid())
	{
		ET_ASSERT(false, "deserializing JSON object failed!");
		return;
	}
}


} // namespace core
} // namespace et
