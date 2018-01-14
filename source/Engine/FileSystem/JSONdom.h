#pragma once
#include <string>
#include <vector>
#include "../Helper/AtomicTypes.hpp"
#include "../Math/Vector.hpp"

namespace JSON
{
	enum ValueType : uint8
	{
		JSON_String,
		JSON_Number,
		JSON_Object,
		JSON_Array,
		JSON_Bool,
		JSON_Null
	};
	struct Value;
	typedef std::pair<std::string, Value*> Pair;
	
	struct String;
	struct Number;
	struct Object;
	struct Array;
	struct Bool;
	//ValueTypes
	struct Value
	{
		virtual JSON::ValueType GetType() { return JSON_Null; }
		virtual ~Value() {}
	
		JSON::String* str();
		JSON::Number* num();
		JSON::Object* obj();
		JSON::Array* arr();
		JSON::Bool* b();
	};
	struct String : public Value
	{
		JSON::ValueType GetType() { return JSON_String; }
		std::string value;
	};
	struct Number : public Value
	{
		JSON::ValueType GetType() { return JSON_Number; }
		double value;
	};
	struct Object : public Value
	{
		JSON::ValueType GetType() { return JSON_Object; }
		~Object();
		JSON::Value* operator[] (const std::string &key)
		{
			for (const JSON::Pair &keyVal : value)
			{
				if (keyVal.first == key)return keyVal.second;
			}
			return nullptr;
		}
	
		std::vector<JSON::Pair> value;
	};
	struct Array : public Value
	{
		JSON::ValueType GetType() { return JSON_Array; }
		~Array();
		JSON::Value* operator[] (const uint32 i)
		{
			if(i>(uint32)value.size()) return nullptr;
			return value[i];
		}
		std::vector<std::string> StrArr();
		std::vector<double> NumArr();
	
		std::vector<JSON::Value*> value;
	};
	struct Bool : public Value
	{
		JSON::ValueType GetType() { return JSON_Bool; }
		bool value;
	};
	
	template<typename T>
	bool ApplyNumValue(JSON::Object* obj, T &val, const std::string &name)
	{
		JSON::Value* jval = (*obj)[name];
		if (jval)
		{
			JSON::Number* jnum = jval->num();
			if (jnum)
			{
				val = static_cast<T>(jnum->value);
				return true;
			}
		}
		return false;
	}
	bool ApplyStrValue(JSON::Object* obj, std::string &val, const std::string &name);
	bool ApplyBoolValue(JSON::Object* obj, bool &val, const std::string &name);
	template<uint8 n, class T>
	bool ArrayVector(JSON::Value* val, etm::vector<n, T> &vec) {
		JSON::Array* jvec = val->arr();
		if (jvec && jvec->value.size() >= n)
		{
			vec = etm::vector<n, T>();
			for(uint8 i = 0; i < 2; ++i)
			{
				JSON::Number* jnum = (*jvec)[(uint32)i]->num();
				if (!jnum) return false;
				vec[i] = static_cast<T>(jnum->value);
			}
			return true;
		}
		return false;
	}
}