#pragma once
#include <string>
#include <Windows.h>
#include "BumiiData.h"

namespace DataTypes
{
	class ModelData
	{
	public:
		byte ModelType;
		std::string Model;
		BumiiData Bumii;
	};
}