#pragma once
#include <string>
#include <vector>
#include "../Helper/AtomicTypes.hpp"
#include "../Math/Vector.hpp"

enum JSONvalueType : uint8
{
	JSON_String,
	JSON_Number,
	JSON_Object,
	JSON_Array,
	JSON_Bool,
	JSON_Null
};
struct JSONvalue;
typedef std::pair<std::string, JSONvalue*> JSONpair;

struct JSONstring;
struct JSONnumber;
struct JSONobject;
struct JSONarray;
struct JSONbool;
//ValueTypes
struct JSONvalue
{
	virtual JSONvalueType GetType() { return JSON_Null; }
	virtual ~JSONvalue() {}

	JSONstring* str();
	JSONnumber* num();
	JSONobject* obj();
	JSONarray* arr();
	JSONbool* b();
};
struct JSONstring : public JSONvalue
{
	JSONvalueType GetType() { return JSON_String; }
	std::string value;
};
struct JSONnumber : public JSONvalue
{
	JSONvalueType GetType() { return JSON_Number; }
	double value;
};
struct JSONobject : public JSONvalue
{
	JSONvalueType GetType() { return JSON_Object; }
	~JSONobject();
	JSONvalue* operator[] (const std::string &key)
	{
		for (const JSONpair &keyVal : value)
		{
			if (keyVal.first == key)return keyVal.second;
		}
		return nullptr;
	}

	std::vector<JSONpair> value;
};
struct JSONarray : public JSONvalue
{
	JSONvalueType GetType() { return JSON_Array; }
	~JSONarray();
	JSONvalue* operator[] (const uint32 i)
	{
		if(i>(uint32)value.size()) return nullptr;
		return value[i];
	}
	std::vector<std::string> StrArr();
	std::vector<double> NumArr();

	std::vector<JSONvalue*> value;
};
struct JSONbool : public JSONvalue
{
	JSONvalueType GetType() { return JSON_Bool; }
	bool value;
};

template<typename T>
bool JSONApplyNumValue(JSONobject* obj, T &val, const std::string &name)
{
	JSONvalue* jval = (*obj)[name];
	if (jval)
	{
		JSONnumber* jnum = jval->num();
		if (jnum)
		{
			val = static_cast<T>(jnum->value);
			return true;
		}
	}
	return false;
}
bool JSONApplyStrValue(JSONobject* obj, std::string &val, const std::string &name);
bool JSONApplyBoolValue(JSONobject* obj, bool &val, const std::string &name);
template<uint8 n, class T>
bool JSONArrayVector(JSONvalue* val, etm::vector<n, T> &vec) {
	JSONarray* jvec = val->arr();
	if (jvec && jvec->value.size() >= n)
	{
		vec = etm::vector<n, T>();
		for(uint8 i = 0; i < 2; ++i)
		{
			JSONnumber* jnum = (*jvec)[(uint32)i]->num();
			if (!jnum) return false;
			vec[i] = static_cast<T>(jnum->value);
		}
		return true;
	}
	return false;
}