#pragma once
#include "Vec3fBE.h"
#include "Enemy.h"

namespace MemoryAccess
{

	class EnemyAccess 
	{
		std::map<int, Enemy*> EnemyList;
		std::shared_mutex Mutex;


	public:
		EnemyAccess()
		{

			EnemyList = {};

		}

		void UpdateEnemyAddress(uint64_t baseAddress, bool created)
		{
			int hash = Memory::read_bigEndian4Bytes(baseAddress + 0x408, __FUNCTION__);
			if (hash == 0)
				return;

			Mutex.lock();

			// Process Enemy to obtain healthAddress and hash;
			if (!EnemyList.count(hash))
				EnemyList[hash] = new Enemy(created ? baseAddress : 0);
			else
				EnemyList[hash]->SetAddress(created ? baseAddress : 0);

			Mutex.unlock();
		}

		void RemoveEnemyFromList(uint64_t baseAddress)
		{
			Mutex.lock();

			EnemyList.clear();

			Mutex.unlock();
		}

		void SetServerData(DTO::EnemyDTO* serverData)
		{
			Mutex.lock();

			for (int i = 0; i < serverData->Health.size(); i++)
			{
				DataTypes::EnemyData svEnemy = serverData->Health[i];

				if (EnemyList.count(svEnemy.Hash) == 0)
					EnemyList[svEnemy.Hash] = new Enemy(0);

				EnemyList[svEnemy.Hash]->SetHealth(svEnemy.Health);
			}

			Mutex.unlock();
		}

		DTO::EnemyDTO* UpdateHealth()
		{
			DTO::EnemyDTO* result = new DTO::EnemyDTO();
			result->Health = {};

			Mutex.lock();

			for (auto const& pair : EnemyList)
			{
				int LocalHash = pair.first;
				Enemy* LocalEnemy = pair.second;

				if (!LocalEnemy->IsSpawned)
					continue;

				if (!LocalEnemy->GetSetup())
					continue;

				LocalEnemy->GetHealth(__FUNCTION__);

				if (!LocalEnemy->IsUpdated)
					continue;
				
				EnemyData* EnemyToAdd = new EnemyData();
				EnemyToAdd->Hash = LocalHash;
				EnemyToAdd->Health = LocalEnemy->CurrentHealth;
				result->Health.push_back(*EnemyToAdd);
				LocalEnemy->IsUpdated = false;
			}

			Mutex.unlock();

			return result;
		}
	};

}

