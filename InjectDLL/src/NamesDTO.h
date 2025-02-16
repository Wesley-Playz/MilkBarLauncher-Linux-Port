#pragma once
#include <map>
#include <string>
#include <Windows.h>

namespace DTO
{

	class NamesDTO
	{
	public:
		std::map<byte, std::string> Names;
	};

}