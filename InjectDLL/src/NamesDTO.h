#pragma once
#include <map>
#include <string>

namespace DTO
{

	class NamesDTO
	{
	public:
		std::map<std::byte, std::string> Names;
	};

}