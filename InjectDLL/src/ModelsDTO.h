#pragma once
#include <map>
#include <string>
#include <Windows.h>
#include "ModelData.h"

using namespace DataTypes;

namespace DTO
{
	class ModelsDTO
	{
	public:
		std::map<byte, ModelData> Models;
	};
}