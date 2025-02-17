#pragma once
#include <map>
#include <string>
#include "ModelData.h"

using namespace DataTypes;

namespace DTO
{
	class ModelsDTO
	{
	public:
		std::map<std::byte, ModelData> Models;
	};
}