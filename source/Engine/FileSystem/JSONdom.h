#pragma once
#include <string>
#include <vector>
#include "../Helper/AtomicTypes.hpp"

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
	~JSONobject() { for (auto el : value)delete el.second; }
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
	~JSONarray() { for (auto el : value)delete el; }
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
