#include "stdafx.h"
#include "serialization.h"

#include <EtCore/Hashing/HashString.h>
#include <EtCore/Content/AssetPointer.h>
#include <EtCore/Content/AssetRegistration.h>

#include "registerMath.h"


namespace et {
namespace core {

namespace serialization {


// Serialization
//////////////////////

	
//---------------------------------
// ToJsonRecursive
//
// Recursively convert an rttr::instance to a json object - returns false if any properties fail to serialize
//
bool ToJsonRecursive(const rttr::instance& inst, JSON::Value*& outJVal, rttr::type const& callingType)
{
	JSON::Object* outJObject = new JSON::Object();
	outJVal = outJObject;

	JSON::Object* appendJObject = outJObject;

	// pointers are wrapped into another object layour to allow polymorphism
	if (callingType.is_pointer())
	{
		rttr::type internalPointerType = inst.get_derived_type().get_raw_type();

		JSON::Pair keyVal = std::make_pair(internalPointerType.get_name().to_string(), new JSON::Object());

		outJObject->value.emplace_back(keyVal);

		appendJObject = keyVal.second->obj();
	}

	rttr::instance instObj = inst.get_type().get_raw_type().is_wrapper() ? inst.get_wrapped_instance() : inst;

	rttr::array_range<rttr::property> const propList = inst.get_derived_type().get_properties();

	bool allPropertiesSerialized = true;

	for (rttr::property const& prop : propList)
	{
		if (prop.get_metadata("NO_SERIALIZE")) // read only should not be serialized probably
		{
			continue;
		}

		rttr::variant const& propVal = prop.get_value(inst);
		if (!propVal)
		{
			continue; // cannot serialize, because we cannot retrieve the value - maybe handle nullptr here
		}

		JSON::Pair keyVal = std::make_pair(prop.get_name().to_string(), nullptr);

		if (!VariantToJsonValue(propVal, keyVal.second))
		{
			LOG("ToJsonRecursive > Failed to serialize property '" + keyVal.first + std::string("' !"), LogLevel::Warning);
			allPropertiesSerialized = false;
		}
		else
		{
			appendJObject->value.push_back(keyVal);
		}
	}

	return allPropertiesSerialized;
}

//---------------------------------
// VariantToJsonValue
//
// Recursively convert an rttr::variant to a json value, figuring out it's type in the process
//
bool VariantToJsonValue(rttr::variant const& var, JSON::Value*& outVal)
{
	rttr::type valueType = var.get_type();
	rttr::type wrappedType = valueType.is_wrapper() ? valueType.get_wrapped_type() : valueType;

	bool isWrapper = wrappedType != valueType;

	if (valueType.is_pointer())
	{
		void* val = (isWrapper ? var.extract_wrapped_value() : var).get_value<void*>();
		if (val == nullptr)
		{
			outVal = new JSON::Value();
			return true;
		}
	}

	if (AtomicTypeToJsonValue(wrappedType, isWrapper ? var.extract_wrapped_value() : var, outVal))
	{
		// check here if this was a nullptr, if it wasn't and the JSON object is of type JSON_Null we should return false
	}
	else if (var.is_sequential_container())
	{
		if (!ArrayToJsonArray(var.create_sequential_view(), outVal))
		{
			LOG("VariantToJsonValue > Failed to convert variant to json array, typeName: '" + wrappedType.get_name().to_string()
				+ std::string("'!"), LogLevel::Warning);

			return false;
		}
	}
	else if (var.is_associative_container())
	{
		if (!AssociativeContainerToJsonArray(var.create_associative_view(), outVal))
		{
			LOG("VariantToJsonValue > Failed to convert variant to associate view, typeName: '" + wrappedType.get_name().to_string() 
				+ std::string("'!"), LogLevel::Warning);

			return false;
		}
	}
	else
	{
		if (!ToJsonRecursive(var, outVal, wrappedType))
		{
			LOG("VariantToJsonValue > Failed to convert variant to JSON object, typeName: '" + wrappedType.get_name().to_string()
				+ std::string("'!"), LogLevel::Warning);

			return false;
		}
		//if (!(wrappedType.get_properties().empty())) // try converting the variant to a JSON object
		//{
		//}
		//else // if that fails, try converting it to a string
		//{
		//	bool stringConversionSuccess = false;
		//	std::string text = var.to_string(&stringConversionSuccess);

		//	if (!stringConversionSuccess)
		//	{
		//		LOG("VariantToJsonValue > Failed to convert variant to JSON string, result: '" + text + std::string("' typeName: '")
		//			+ wrappedType.get_name().to_string() + std::string("'!"), LogLevel::Warning);

		//		return false;
		//	}

		//	outVal = new JSON::String();
		//	outVal->str()->value = text;
		//}
	}

	return true;
}

//---------------------------------
// IsVectorType
//
bool IsVectorType(rttr::type const type)
{
	rttr::variant val = type.get_metadata(MathMeta::s_VectorType);
	return val.is_valid() && val.get_value<bool>();
}

//---------------------------------
// AtomicTypeToJsonValue
//
// Convert a handful of basic supported "leaf" types to Atomic JSON types. Might have to add more in the future. 
// If value type is not atomic it returns false. If value type is atomic but not supported it returns true and outVal will be of type JSON_Null
//
bool AtomicTypeToJsonValue(rttr::type const& valueType, rttr::variant const& var, JSON::Value*& outVal)
{
	if (valueType.is_arithmetic()) // basic arithmetic types get converted to numbers or bools
	{
		if (valueType == rttr::type::get<bool>())
		{
			JSON::Bool* jBool = new JSON::Bool();
			jBool->value = var.to_bool();
			outVal = jBool;
		}
		else if (valueType == rttr::type::get<char>())
		{
			// store char as number because otherwise it would get picked up as a string
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint8());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<int8>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_int8());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<int16>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_int16());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<int32>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_int32());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<int64>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = var.to_int64();
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<uint8>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint8());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<uint16>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint16());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<uint32>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint32());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<uint64>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = static_cast<int64>(var.to_uint64());
			jNum->isInt = true;
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<float>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->value = var.to_double();
			outVal = jNum;
		}
		else if (valueType == rttr::type::get<double>())
		{
			JSON::Number* jNum = new JSON::Number();
			jNum->value = var.to_double();
			outVal = jNum;
		}
		else
		{
			LOG("AtomicTypeToJsonValue > Atomic type not supported, typeName: '" + valueType.get_name().to_string() + std::string("'!")
				, LogLevel::Warning);

			outVal = new JSON::Value(); // return true because it's definitly an arithmetic type but make it null
		}

		return true;
	}
	else if (valueType.is_enumeration()) // enumeration to JSON
	{
		// first try converting it to a string
		bool conversionSuccess = false;
		std::string stringVal = var.to_string(&conversionSuccess);

		if (conversionSuccess)
		{
			JSON::String* jString = new JSON::String();
			jString->value = stringVal;
			outVal = jString;
		}
		else
		{
			// if that doesn't work we convert it to a uint64 instead
			uint64 intVal = var.to_uint64(&conversionSuccess);

			if (conversionSuccess)
			{
				JSON::Number* jNum = new JSON::Number();
				jNum->valueInt = static_cast<int64>(intVal);
				jNum->isInt = true;
				outVal = jNum;
			}
			else
			{
				// That failed too
				// return true because it's definitly an arithmetic type but make it null

				LOG("AtomicTypeToJsonValue > Enum failed to convert, typeName: '" + valueType.get_name().to_string() + std::string("'!")
					, LogLevel::Warning);

				outVal = new JSON::Value();
			}
		}

		return true;
	}
	else if (valueType == rttr::type::get<std::string>()) // string to json
	{
		JSON::String* jString = new JSON::String();
		jString->value = var.to_string();
		outVal = jString;
		return true;
	}
	else if (valueType == rttr::type::get<HashString>()) // hash string to json - depending on the build we store strings or integers
	{
		ET_ASSERT(var.is_type<HashString>());
		HashString const& hash = var.get_value<HashString>();

#if ET_HASH_STRING_ENABLED
		char const* const str = hash.GetStoredString();
		if (str != nullptr)
		{
			ET_ASSERT(hash.Get() == GetHash(str));

			JSON::String* jString = new JSON::String();
			jString->value = str;
			outVal = jString;
			return true;
		}
		else
		{
#endif
			JSON::Number* jNum = new JSON::Number();
			jNum->valueInt = hash.Get();
			jNum->isInt = true;
			outVal = jNum;
			return true;
#if ET_HASH_STRING_ENABLED
		}
#endif
	}
	else if (IsVectorType(valueType))
	{
		rttr::array_range<rttr::property> const prop_list = valueType.get_properties();
		auto prop = *prop_list.begin();
		ET_ASSERT(prop_list.size() == 1);
		ET_ASSERT(prop.get_name().to_string() == "data");

		rttr::instance inst(var);
		rttr::type propValType = prop.get_type();
		rttr::variant propVar = prop.get_value(inst);

		ET_ASSERT(propValType.is_sequential_container());

		if (ArrayToJsonArray(propVar.create_sequential_view(), outVal))
		{
			return true;
		}
		else
		{
			LOG("VariantToJsonValue > Failed to convert vector type to json array, typeName: '" + valueType.get_name().to_string()
				+ std::string("'!"), LogLevel::Warning);
		}
	}
	else if (IsSerializableAssetPointerType(valueType))
	{
		I_AssetPtr const ptr = var.get_value<I_AssetPtr>();
		JSON::String* jString = new JSON::String();

		if (!ptr.is_null())
		{
			jString->value = ptr.GetAsset()->GetName();
		}

		outVal = jString;
		return true;
	}

	// couldn't handle type, return false to indicate that this should be an object or array
	return false;
}

//---------------------------------
// ArrayToJsonArray
//
// Converts a sequential view to a JSON array
//
bool ArrayToJsonArray(const rttr::variant_sequential_view& view, JSON::Value*& outVal)
{
	JSON::Array* outArr = new JSON::Array();
	outVal = outArr;

	bool allItemsSucceeded = true;
	
	for (const auto& item : view)
	{
		JSON::Value* jItem = nullptr;

		if (item.is_sequential_container()) // inner array
		{
			if (!ArrayToJsonArray(item.create_sequential_view(), jItem))
			{
				LOG("ArrayToJsonArray > failed to convert array element to inner json array, typeName: '" + 
					item.get_type().get_name().to_string() + std::string("'!"), LogLevel::Warning);

				allItemsSucceeded = false;
			}
		}
		else // atomic type or object
		{
			rttr::variant wrappedVar = item.extract_wrapped_value();
			rttr::type valueType = wrappedVar.get_type();
			if (valueType.is_pointer())
			{
				void* val = wrappedVar.get_value<void*>();
				if (val == nullptr)
				{
					jItem = new JSON::Value();
					outArr->value.emplace_back(jItem);
					continue;
				}
			}

			if (valueType.is_arithmetic() 
				|| (valueType == rttr::type::get<std::string>())
				|| valueType.is_enumeration() 
				|| (valueType == rttr::type::get<HashString>())
				|| IsVectorType(valueType)
				|| IsSerializableAssetPointerType(valueType))
			{
				if (!AtomicTypeToJsonValue(valueType, wrappedVar, jItem))
				{
					LOG("ArrayToJsonArray > failed to convert array element to atomic type, typeName: '" + valueType.get_name().to_string() 
						+ std::string("'!"), LogLevel::Warning);

					allItemsSucceeded = false;
				}
			}
			else // object
			{
				if (!ToJsonRecursive(wrappedVar, jItem, valueType))
				{
					LOG("ArrayToJsonArray > failed to convert array element to json object, typeName: '" + valueType.get_name().to_string()
						+ std::string("'!"), LogLevel::Warning);

					allItemsSucceeded = false;
				}
			}
		}

		outArr->value.emplace_back(jItem);
	}

	return allItemsSucceeded;
}

//---------------------------------
// AssociativeContainerToJsonArray
//
// Converts a associative view to a JSON array of objects containing keys and values. If any item fails to be serialized this returns false
//
bool AssociativeContainerToJsonArray(const rttr::variant_associative_view& view, JSON::Value*& outVal)
{
	static const std::string keyName("key");
	static const std::string valueName("value");

	JSON::Array* outArr = new JSON::Array();
	outVal = outArr;

	bool allItemsSucceeded = true;

	if (view.is_key_only_type()) // treat this as a normal array
	{
		for (auto& item : view)
		{
			JSON::Value* jItem = nullptr;

			if (!VariantToJsonValue(item.first, jItem))
			{
				LOG("ArrayToJsonArray > failed to convert key only variant to json value!", LogLevel::Warning);
				allItemsSucceeded = false;
			}

			outArr->value.emplace_back(jItem);
		}
	}
	else // treat this as a objects with keys and values
	{
		for (auto& item : view)
		{
			JSON::Object* jObj = new JSON::Object();
			
			// create the key element and place it in the object
			JSON::Pair keyPair = std::make_pair(keyName, nullptr);
			if (!VariantToJsonValue(item.first, keyPair.second))
			{
				LOG("ArrayToJsonArray > failed to convert key variant to json value!", LogLevel::Warning);
				allItemsSucceeded = false;
			}
			jObj->value.emplace_back(keyPair);

			// create the value element and place it in the object
			JSON::Pair valuePair = std::make_pair(valueName, nullptr);
			if (!VariantToJsonValue(item.second, valuePair.second))
			{
				LOG("ArrayToJsonArray > failed to convert value variant to json value!", LogLevel::Warning);
				allItemsSucceeded = false;
			}
			jObj->value.emplace_back(valuePair);

			// place the key value object in the array
			outArr->value.emplace_back(jObj);
		}
	}

	return allItemsSucceeded;
}


// Deserialization
//////////////////////


//---------------------------------
// ExtractBasicTypes
//
// Convert basic JSON "leaf" values to rttr::variants, assuming the variant will convert it to its actual type later
// If the JSON value is not a basic type we return an invalid variant
//
rttr::variant ExtractBasicTypes(JSON::Value const* const jVal)
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

//---------------------------------
// ArrayFromJsonRecursive
//
// deserialize a JSON array into an rttr sequential view
//  - returns false if any of the elements failes deserialization, but tries to parse the entire view anyway
//
bool ArrayFromJsonRecursive(rttr::variant_sequential_view& view, JSON::Value const* const jVal)
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

//---------------------------------
// ExtractValue
//
// Extracts a json basic type or object to an rttr variant, if it is neither it will return an invalid variant
//
rttr::variant ExtractValue(JSON::Value const* const jVal, const rttr::type& valueType)
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

//---------------------------------
// AssociativeViewFromJsonRecursive
//
// deserialize a JSON array into an rttr associative view (dictionary / map)
//  - returns false if any of the key value pairs failes deserialization, but tries to parse the entire view anyway
//
bool AssociativeViewFromJsonRecursive(rttr::variant_associative_view& view, JSON::Value const* const jVal)
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
// FromJsonValue
//
// Extract a JSON Value into a variant using its value type
//
void FromJsonValue(JSON::Value const* jVal, rttr::type &valueType, rttr::variant &var)
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

//---------------------------------
// ExtractPointerValueType
//
// Pointer objects are wrapped so that they can indicate their underlying inherited type
//
bool ExtractPointerValueType(rttr::type &inOutValType, JSON::Value const* &inOutJVal, bool& isNull)
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

//---------------------------------
// ObjectFromJsonRecursive
//
// Recursively deserialize JSON values into an object or pointer(the instance) with a known type
//
void ObjectFromJsonRecursive(JSON::Value const* const jVal, rttr::instance const &inst, rttr::type &instType)
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

//---------------------------------
// FromJsonRecursive
//
// Recursively deserialize JSON values into an object or pointer(the instance)
//
void FromJsonRecursive(rttr::instance const inst, JSON::Object const* const parentObj) // assumes jVal is a JSON::Object
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


} // namespace serialization

} // namespace core
} // namespace et
