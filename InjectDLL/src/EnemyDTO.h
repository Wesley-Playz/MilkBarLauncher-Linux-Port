#pragma once
#include "EnemyData.h"
#include <vector>

using namespace DataTypes;

namespace DTO
{

	class EnemyDTO
	{
	public:
		std::vector<EnemyData> Health;
	};

}