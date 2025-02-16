#pragma once

#include "Connectivity.h"

rapidjson::Document Connectivity::deserializeServerData(std::string message)
{

	rapidjson::Document doc;
	doc.Parse(message.c_str());

	rapidjson::StringBuffer buf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buf);

	doc.Accept(writer);

	return doc;

}