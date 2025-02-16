#pragma once

#include "CharacterDTO.h"
#include "WorldDTO.h"
#include "EnemyDTO.h"
#include "QuestDTO.h"
#include "BombDTO.h"

namespace DTO
{

	class ClientDTO
	{
	public:
		WorldDTO* WorldData;
		ClientCharacterDTO* PlayerData;
		EnemyDTO* EnemyData;
		QuestDTO* QuestData;
	};

}