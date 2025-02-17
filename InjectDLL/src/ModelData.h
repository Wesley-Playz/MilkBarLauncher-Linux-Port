#pragma once
#include <string>
#include "BumiiData.h"

namespace DataTypes
{
	class ModelData
	{
	public:
		std::byte ModelType;
		std::string Model;
		BumiiData Bumii;
	};
}