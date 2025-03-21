#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <cstdint>   // For uint32_t, uint64_t
#include <shared_mutex> // For std::shared_mutex
#include "Connectivity.h"
#include "Memory.h"
#include "LoggerService.h"

namespace Main
{

	//extern Logging::LoggerService* Logger;

	extern uint64_t baseAddr;

	extern uint32_t t0;
	extern uint32_t t1;
	extern char message[];
	extern std::string serverData;
	extern std::string serverName;
	extern int playerNumber;
	extern bool IsProp;
	extern bool IsPropHuntStopped;
	extern bool HidePlayer32;
	extern std::vector<uint8_t> FoundPlayers;
	extern std::vector<bool> ConnectedPlayers;

	extern Memory::Link_class* Link;
	extern Memory::World_class* World;

	extern Memory::OtherPlayer_class* Jugador1;
	extern Memory::OtherPlayer_class* Jugador2;
	extern Memory::OtherPlayer_class* Jugador3;
	extern Memory::OtherPlayer_class* Jugador4;

	extern Memory::OtherPlayer_class* Jugadores[];

	extern Memory::BombSyncer* BombSync;
	extern bool QuestSyncReady;

	extern std::vector<std::vector<float>> Jugador1Queue;
	extern std::vector<std::vector<float>> Jugador2Queue;
	extern std::vector<std::vector<float>> Jugador3Queue;
	extern std::vector<std::vector<float>> Jugador4Queue;

	extern std::vector<std::vector<float>> JugadoresQueues[];

	extern Connectivity::Client* client;

	extern std::shared_mutex WeaponChangeMutex;
	extern std::shared_mutex BombExplodeMutex;

	extern std::vector<float> oldLocations[];

	extern float targetFPS;
	extern float serializationRate;
	extern float ping;

	////// Server properties //////
	extern bool isEnemySync;
	extern bool isGlyphSync;
	extern bool isQuestSync;
	extern bool isHvsSR;
	extern bool isDeathSwap;

	extern int GlyphUpdateTime;
	extern int GlyphDistance;

	extern std::vector<std::string> questServerSettings;

	extern bool isPaused;

}
