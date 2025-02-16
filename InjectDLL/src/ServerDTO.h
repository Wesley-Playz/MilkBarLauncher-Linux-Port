#pragma once

#include "CharacterDTO.h"
#include "WorldDTO.h"
#include "NamesDTO.h"
#include "ModelsDTO.h"
#include "EnemyDTO.h"
#include "QuestDTO.h"
#include "NetworkDTO.h"
#include "DeathSwapDTO.h"
#include "TeleportDTO.h"
#include "PropHuntDTO.h"

namespace DTO
{
	class ServerDTO
	{
	public:
		WorldDTO* WorldData;
		NamesDTO* NameData;
		ModelsDTO* ModelData;
		std::vector<CloseCharacterDTO*> ClosePlayers;
		std::vector<FarCharacterDTO*> FarPlayers;
		EnemyDTO* EnemyData;
		QuestDTO* QuestData;
		NetworkDTO* NetworkData;
		DeathSwapDTO* DeathSwapData;
		TeleportDTO* TeleportData;
		PropHuntDTO* PropHuntData;
	};

}