#pragma once
#include <string>
#include <vector>
#include "../Helper/AtomicTypes.hpp"
#include "../Math/Vector.hpp"
#include "../Math/Matrix.hpp"

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
		int64 valueInt;
		bool isInt = false;
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
		std::vector<int64> IntArr();
	
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
	template<typename T>
	bool ApplyIntValue(JSON::Object* obj, T &val, const std::string &name)
	{
		JSON::Value* jval = (*obj)[name];
		if (jval)
		{
			JSON::Number* jnum = jval->num();
			if (jnum)
			{
				if (!(jnum->isInt))return false;
				val = static_cast<T>(jnum->valueInt);
				return true;
			}
		}
		return false;
	}
	bool ApplyStrValue(JSON::Object* obj, std::string &val, const std::string &name);
	bool ApplyBoolValue(JSON::Object* obj, bool &val, const std::string &name);
	template<uint8 n, class T>
	bool ArrayVector(JSON::Value* val, etm::vector<n, T> &vec) 
	{
		if (!(val->GetType() == ValueType::JSON_Array)) return false;
		JSON::Array* jvec = val->arr();
		if (jvec && jvec->value.size() >= n)
		{
			vec = etm::vector<n, T>();
			for(uint8 i = 0; i < n; ++i)
			{
				JSON::Number* jnum = (*jvec)[(uint32)i]->num();
				if (!jnum) return false;
				vec[i] = static_cast<T>(jnum->value);
			}
			return true;
		}
		return false;
	}
	template<uint8 n, uint8 m, class T>
	bool ArrayMatrix(JSON::Value* val, etm::matrix<n, m, T> &mat) 
	{
		if (!(val->GetType() == ValueType::JSON_Array)) return false;
		JSON::Array* jvec = val->arr();
		if (jvec && jvec->value.size() >= n*m)
		{
			mat = etm::matrix<n, m, T>(etm::uninitialized);
			for (uint8 i = 0; i < m; ++i)
			{
				for (uint8 j = 0; j < n; ++j)
				{
					JSON::Number* jnum = (*jvec)[(uint32)i*m+j]->num();
					if (!jnum) return false;
					mat[i][j] = static_cast<T>(jnum->value);
				}
			}
			return true;
		}
		return false;
	}
}