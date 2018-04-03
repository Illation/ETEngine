#include "stdafx.hpp"
#include "JSONdom.h"

JSON::String* JSON::Value::str()
{
	if (GetType() == JSON_String)return static_cast<JSON::String*>(this); return nullptr;
}

JSON::Number* JSON::Value::num()
{
	if (GetType() == JSON_Number)return static_cast<JSON::Number*>(this); return nullptr;
}

JSON::Object::~Object()
{
	for (uint32 i = 0; i<(uint32)value.size(); ++i)
	{
		delete value[i].second;
	}
}

JSON::Object* JSON::Value::obj()
{
	if (GetType() == JSON_Object)return static_cast<JSON::Object*>(this); return nullptr;
}

JSON::Array::~Array()
{
	for (uint32 i = 0; i < (uint32)value.size(); ++i)
	{
		delete value[i];
	}
}

JSON::Array* JSON::Value::arr()
{
	if (GetType() == JSON_Array)return static_cast<JSON::Array*>(this); return nullptr;
}

JSON::Bool* JSON::Value::b()
{
	if (GetType() == JSON_Bool)return static_cast<JSON::Bool*>(this); return nullptr;
}

std::vector<std::string> JSON::Array::StrArr()
{
	std::vector<std::string>ret;
	for (auto el : value)
	{
		if (!(el->GetType() == JSON::ValueType::JSON_String)) continue;
		JSON::String* jstr = el->str();
		if (jstr)ret.push_back(jstr->value);
	}
	return ret;
}

std::vector<double> JSON::Array::NumArr()
{
	std::vector<double>ret;
	for (auto el : value)
	{
		if (!(el->GetType() == JSON::ValueType::JSON_Number)) continue;
		JSON::Number* jnum = el->num();
		if (jnum)ret.push_back(jnum->value);
	}
	return ret;
}

std::vector<int64> JSON::Array::IntArr()
{
	std::vector<int64>ret;
	for (auto el : value)
	{
		if (!(el->GetType() == JSON::ValueType::JSON_Number)) continue;
		JSON::Number* jnum = el->num();
		if (jnum)ret.push_back(jnum->valueInt);
	}
	return ret;
}

bool JSON::ApplyStrValue(JSON::Object* obj, std::string &val, const std::string &name)
{
	JSON::Value* jval = (*obj)[name];
	if (jval)
	{
		JSON::String* jstr = jval->str();
		if (jstr)
		{
			val = jstr->value;
			return true;
		}
	}
	return false;
}

bool JSON::ApplyBoolValue(JSON::Object* obj, bool &val, const std::string &name)
{
	JSON::Value* jval = (*obj)[name];
	if (jval)
	{
		JSON::Bool* jbool = jval->b();
		if (jbool)
		{
			val = jbool->value;
			return true;
		}
	}
	return false;
}
