#pragma once

#include "Connectivity.h"
#include <vector>
#include <iostream>


rapidjson::Value Connectivity::addValueToJsonDocument(rapidjson::Document::AllocatorType& allocator, std::string data, std::string dataType)
{

	rapidjson::Value val(rapidjson::kObjectType);


	if (dataType == "int")
	{
		val.SetInt(std::stoi(data));
	}
	else if (dataType == "float")
	{
		val.SetFloat(std::stof(data));
	}
	else if (dataType == "string")
	{
		val.SetString(data.c_str(), static_cast<rapidjson::SizeType>(data.length()), allocator);
	}

	//doc.AddMember(rapidjson::StringRef(dataName), val, allocator);

	return val;

}

rapidjson::Value Connectivity::addMapToJsonDocument(rapidjson::Document::AllocatorType& allocator, std::map<std::string, std::string> data, std::string dataTypes)
{

	rapidjson::Value map(rapidjson::kArrayType);
	rapidjson::Value obj(rapidjson::kObjectType);
	rapidjson::Value val(rapidjson::kObjectType);

	for (auto const& pair : data)
	{

		std::string DName = pair.first;
		std::string DValue = pair.second;

		if (dataTypes == "int")
		{
			val.SetInt(std::stoi(DValue));
		}
		else if (dataTypes == "float")
		{
			val.SetFloat(std::stof(DValue));
		}
		else if (dataTypes == "string")
		{
			val.SetString(DValue.c_str(), static_cast<rapidjson::SizeType>(DValue.length()), allocator);
		}

		obj.AddMember(rapidjson::Value(DName, allocator).Move(), val, allocator);

	}

	//map.PushBack(obj, allocator);

	return obj;
	//doc.AddMember(rapidjson::StringRef(dataName), map, allocator);

}

rapidjson::Value Connectivity::addVectorToJsonDocument(rapidjson::Document::AllocatorType& allocator, std::vector<std::string> data, std::string dataTypes)
{

	rapidjson::Value vector(rapidjson::kArrayType);
	rapidjson::Value val(rapidjson::kObjectType);

	for (int i = 0; i < data.size(); i++)
	{

		if (dataTypes == "int")
		{
			val.SetInt(std::stoi(data[i]));
		}
		else if (dataTypes == "float")
		{
			val.SetFloat(std::stof(data[i]));
		}
		else if (dataTypes == "string")
		{
			val.SetString(data[i].c_str(), static_cast<rapidjson::SizeType>(data[i].length()), allocator);
		}

		vector.PushBack(val, allocator);

	}

	return vector;

}