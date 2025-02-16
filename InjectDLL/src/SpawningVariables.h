#pragma once

#include <shared_mutex>
#include "KeyCodeActor.h"
#include "BotwEdit.h"
#include <map>
#include "ActorData.h"
#include "Memory.h"
#include "dllmain_Variables.h"
#include "Entities.h"
#include "Game.h"

// This stuff here was yoinked from BetterVR
// -----------------------------------------
	extern union FPR_t {
		double fpr;
		struct
		{
			double fp0;
			double fp1;
		};
		struct
		{
			uint64_t guint;
		};
		struct
		{
			uint64_t fp0int;
			uint64_t fp1int;
		};
	};

	extern struct PPCInterpreter_t {
		uint32_t instructionPointer;
		uint32_t gpr[32];
		FPR_t fpr[32];
		uint32_t fpscr;
		uint8_t crNew[32]; // 0 -> bit not set, 1 -> bit set (upper 7 bits of each byte must always be zero) (cr0 starts at index 0, cr1 at index 4 ..)
		uint8_t xer_ca;  // carry from xer
		uint8_t LSQE;
		uint8_t PSE;
		// thread remaining cycles
		int32_t remainingCycles; // if this value goes below zero, the next thread is scheduled
		int32_t skippedCycles; // if remainingCycles is set to zero to immediately end thread execution, this value holds the number of skipped cycles
		struct
		{
			uint32_t LR;
			uint32_t CTR;
			uint32_t XER;
			uint32_t UPIR;
			uint32_t UGQR[8];
		}sprNew;
	};
	// -----------------------------------------

	typedef void (*osLib_registerHLEFunctionType)(const char* libraryName, const char* functionName, void(*osFunction)(PPCInterpreter_t* hCPU));


#pragma pack(1)
	extern struct TransferableData { // This is reversed compared to the gfx pack because we read as big endian.
		int f_r10;
		int f_r9;
		int f_r8;
		int f_r7;
		int f_r6;
		int f_r5;
		int f_r4;
		int f_r3;

		int ringPtr;

		int fnAddr;

		byte bytepadding[2];
		bool interceptRegisters;

		bool enabled;
	};

#pragma pack(1)
	extern struct InstanceData {
		char name[152]; // We'll allocate all unused storage for use for name storage.. just in case of a really long actor name
		uint8_t actorStorage[104];
	};

	extern struct QueueActor {
		float PosX;
		float PosY;
		float PosZ;
		std::string Name;
	};

// ---------------------------------------------------------------------------------
// This is an example function call.
// - Feel free to expand / change -
// Note: This does not take into account stuff like actually setting desired params.
// ---------------------------------------------------------------------------------
std::map<char, std::vector<KeyCodeActor>> keyCodeMap;

std::shared_mutex keycode_mutex;
std::shared_mutex queue_mutex;
std::shared_mutex data_mutex;

std::map<char, bool> prevKeyStateMap; // Used for key press logic - keeps track of previous key state

std::vector<QueueActor> queuedActors;

MemoryInstance* memInstance;

bool isSetup = false;

void setup(PPCInterpreter_t* hCPU, uint32_t startTrnsData, uint64_t baseAddress) {

	TransferableData trnsData;
	data_mutex.lock(); //////////////////////////////////////////////////
	memInstance->memory_readMemoryBE(startTrnsData, &trnsData, baseAddress); // Just make sure to intercept stuff..
	trnsData.interceptRegisters = true;
	memInstance->memory_writeMemoryBE(startTrnsData, trnsData, baseAddress);
	data_mutex.unlock(); //===============================================

	isSetup = true;
}

void queueActor(int playerNumber, float Position[3])
{
	if (queuedActors.size() > 0)
		for (int i = 0; i < queuedActors.size(); i++)
			if (queuedActors[i].Name == "Jugador" + std::to_string(playerNumber))
				return;

	//for (int j = queuedActors.size() - 1; j >= 0; j--)
	//{
	//	if (queuedActors[j].Name == "Jugador" + std::to_string(playerNumber))
	//	{
	//		return;
	//	}
	//}

	QueueActor queueActor;
	std::string actorName = "Jugador";
	actorName.append(std::to_string(playerNumber));
	queueActor.Name = actorName;
	queueActor.PosX = Position[0];
	queueActor.PosY = Position[1];
	queueActor.PosZ = Position[2];

	queuedActors.push_back(queueActor);
}

void queueActor(std::string actorName, float Position[3])
{

	QueueActor queueActor;
	queueActor.Name = actorName;
	queueActor.PosX = Position[0];
	queueActor.PosY = Position[1];
	queueActor.PosZ = Position[2];

	queuedActors.push_back(queueActor);

}

void setupActor(PPCInterpreter_t* hCPU, TransferableData& trnsData, InstanceData& instData, uint32_t startRingBuffer, uint32_t endRingBuffer, uint64_t baseAddress) {
	QueueActor qAct = queuedActors[0];


	// Lets set any data that our params will reference:
	// -------------------------------------------------

	// Copy needed data over to our own storage to not override actor stuff
	data_mutex.lock_shared(); //////////////////////////////////////////////////
	memInstance->memory_readMemoryBE(trnsData.f_r7, &instData.actorStorage, baseAddress);
	data_mutex.unlock_shared(); //==============================================

	int actorStorageLocation = trnsData.ringPtr + sizeof(instData) - sizeof(instData.name) - sizeof(instData.actorStorage);
	int mubinLocation = trnsData.ringPtr + sizeof(instData) - sizeof(instData.name) - sizeof(instData.actorStorage) + (7 * 4); // The MubinIter lives inside the actor btw

	// Set actor pos to stored pos
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (15 * 4)], &qAct.PosX, sizeof(float));
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (16 * 4)], &qAct.PosY, sizeof(float));
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (17 * 4)], &qAct.PosZ, sizeof(float));

	// We want to make sure there's a fairly high traverseDist
	float traverseDist = 0.f; // Hmm... this kinda proves this isn't really used
	short traverseDistInt = (short)traverseDist;
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (18 * 4)], &traverseDist, sizeof(float));
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (37 * 2)], &traverseDistInt, sizeof(short));

	int null = 0;
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (11 * 4)], &null, sizeof(int)); // mLinkData

	// Might as well null out some other things
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (9 * 4)], &null, sizeof(int)); // mData
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (10 * 4)], &null, sizeof(int)); // mProc
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (7 * 4)], &null, sizeof(int)); // idk what this is
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (3 * 4)], &null, sizeof(int)); // or this, either



	// Not sure what these are, but they helps with traverseDist issues
	int traverseDistFixer = 0x043B0000;
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (2 * 4)], &traverseDistFixer, sizeof(int));
	int traverseDistFixer2 = 0x00000016;
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (1 * 4)], &traverseDistFixer2, sizeof(int));


	// Oh, and the HashId as well
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (14 * 4)], &null, sizeof(int));

	// And we can make mRevivalGameDataFlagHash an invalid handle
	int invalid = -1;
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (12 * 4)], &invalid, sizeof(int));
	// And whatever this is, too
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (13 * 4)], &invalid, sizeof(int));

	// We can also get rid of this junk
	memcpy(&instData.actorStorage[sizeof(instData.actorStorage) - (8 * 4)], &invalid, sizeof(int));




	// Set name!
	{ // Copy to name string storage... reversed
		int pos = sizeof(instData.name) - 1;
		for (char const& c : qAct.Name) {
			memcpy(instData.name + pos, &c, 1);
			pos--;
		}
		uint8_t nullByte = 0;
		memcpy(instData.name + pos, &nullByte, 1); // Null terminate!
	}

	// -------------------------------------------------

	// Set registers for params and stuff
	hCPU->gpr[3] = trnsData.f_r3;
	hCPU->gpr[4] = trnsData.ringPtr + sizeof(instData) - sizeof(instData.name);
	hCPU->gpr[5] = trnsData.f_r5;
	hCPU->gpr[6] = mubinLocation;
	hCPU->gpr[7] = actorStorageLocation;
	hCPU->gpr[8] = 0;
	hCPU->gpr[9] = 1;
	hCPU->gpr[10] = 0;
	trnsData.fnAddr = 0x037b6040; // Address to call to

	trnsData.enabled = true; // This tells the assembly patch to trigger one function call

	trnsData.interceptRegisters = false; // We don't want to intercept *this* function call

	// Write our actor data!
	data_mutex.lock(); ////////////////////////////////////////////////////
	memInstance->memory_writeMemoryBE(trnsData.ringPtr, instData, baseAddress);
	data_mutex.unlock(); //================================================

	trnsData.ringPtr += sizeof(InstanceData); // Move our ring ptr to the next slot!
	if (trnsData.ringPtr >= endRingBuffer) // If we're at the end of the ring....
trnsData.ringPtr = startRingBuffer; // move to the start!

// Gotta remove this actor from the queue!
queuedActors.erase(queuedActors.begin());
}

void mainFn(PPCInterpreter_t* hCPU, uint32_t startTrnsData, uint32_t startRingBuffer, uint32_t endRingBuffer, uint64_t baseAddress) {
	hCPU->instructionPointer = hCPU->sprNew.LR; // Tell it where to return to - REQUIRED

	// This is the stuff I'm currently using to test different values for potential Link coords
	//MemoryInstance::floatBE* pos = reinterpret_cast<MemoryInstance::floatBE*>(memInstance->baseAddr + 0x10263910);
	//Console::LogPrint((float)*pos);

	if (!isSetup) {
		setup(hCPU, startTrnsData, baseAddress);
		return;
	}

	//queueActors();

	data_mutex.lock_shared(); /////////////////////////////////////////////////////////////////////
	// Get our transferrable data
	TransferableData trnsData;
	memInstance->memory_readMemoryBE(startTrnsData, &trnsData, baseAddress);

	// Get our instance data
	uint32_t startInstData = trnsData.ringPtr;
	InstanceData instData;
	memInstance->memory_readMemoryBE(startInstData, &instData, baseAddress);
	data_mutex.unlock_shared(); //==================================================================

	trnsData.interceptRegisters = true; // Just make sure to intercept stuff.. if we don't do this all the time when you warp somewhere else spawns cause it to crash

	queue_mutex.lock(); ////////////////////////////////////////////
	// Actual actor spawning - just read from queue here.
	if (queuedActors.size() >= 1) {
		setupActor(hCPU, trnsData, instData, startRingBuffer, endRingBuffer, baseAddress);
	}
	queue_mutex.unlock(); //========================================

	data_mutex.lock(); ////////////////////////////////////////////////////////////////////
	memInstance->memory_writeMemoryBE(startTrnsData, trnsData, baseAddress);
	data_mutex.unlock(); //==================================================================
}

// A wrapper function to set the params in a more cpp-friendly way
void mainFn(PPCInterpreter_t* hCPU) {

	hCPU->instructionPointer = hCPU->sprNew.LR; // Tell it where to return to - REQUIRED
	mainFn(hCPU, hCPU->gpr[3], hCPU->gpr[4], hCPU->gpr[5], Main::baseAddr);
}

enum Weather : uint32_t
{
	BlueSky,
	Cloudy,
	Rain,
	HeavyRain,
	Snow,
	HeavySnow,
	Thunderstorm,
	ThunderRain,
	BlueSkyRain
};

Weather newWeather = (Weather)0;

void WeatherFn(PPCInterpreter_t* hCPU) {

	hCPU->instructionPointer = hCPU->sprNew.LR;

	Game::GameInstance->World->LocalWeather = (int)(Weather)hCPU->gpr[26];
	newWeather = (Weather)Game::GameInstance->World->Weather;

	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 0, (char)newWeather, Main::baseAddr);
	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 1, (char)newWeather, Main::baseAddr);
	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 2, (char)newWeather, Main::baseAddr);
	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 3, (char)newWeather, Main::baseAddr);
	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 4, (char)newWeather, Main::baseAddr);
	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 5, (char)newWeather, Main::baseAddr);
	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 6, (char)newWeather, Main::baseAddr);
	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 7, (char)newWeather, Main::baseAddr);
	memInstance->memory_writeMemoryBE(hCPU->gpr[3] + 8, (char)newWeather, Main::baseAddr);
}

int LastCreated = 0;
bool started = false;

void OnActorCreate(PPCInterpreter_t* hCPU)
{
	hCPU->instructionPointer = hCPU->sprNew.LR;

	std::string name = Memory::read_string(Main::baseAddr + hCPU->gpr[3] + 0x10, 100, __FUNCTION__);

	std::vector<std::string> BombChoices = { "CustomRemoteBomb", "CustomRemoteBomb2", "CustomRemoteBombCube", "CustomRemoteBombCube2" };

	if (name.rfind("Enemy_Bokoblin_Junior") != std::string::npos)
	{
		std::stringstream stream;
		stream << "Bokoblin: " << std::hex << hCPU->gpr[3];
		Logging::LoggerService::LogDebug(stream.str());
	}

	if (name.rfind("GameROMPlayer") != std::string::npos)
	{
		std::stringstream stream;
		stream << "Link: " << std::hex << hCPU->gpr[3];
		Logging::LoggerService::LogDebug(stream.str());
		Game::GameInstance->setAddress(hCPU->gpr[3]);
	}

	if (name.rfind("Jugador", 0) == 0)
	{
		int spawnedPlayer = std::stoi(name.replace(0, 7, ""));

		std::stringstream stream;
		stream << "Spawned player " << spawnedPlayer << " at: " << std::hex << Main::baseAddr + hCPU->gpr[3] << ". Setting up addresses.";
		Logging::LoggerService::LogDebug(stream.str(), __FUNCTION__);

		Instances::PlayerList[spawnedPlayer]->setAddress(hCPU->gpr[3]);
		Instances::PlayerList[spawnedPlayer]->Equipment->SetWeapons(hCPU->gpr[3]);
		Instances::PlayerList[spawnedPlayer]->Bumii->setAddress(hCPU->gpr[3]);

		Logging::LoggerService::LogDebug("Player " + std::to_string(spawnedPlayer) + " setup correctly.", __FUNCTION__);
	}

	if (name.rfind("Enemy_", 0) == 0)
		Game::GameInstance->EnemyService->UpdateEnemyAddress(Main::baseAddr + hCPU->gpr[3], true);

	if (!started) return;

	if (name == "RemoteBomb")
	{
		Game::GameInstance->Bomb->setAddress(hCPU->gpr[3], __FUNCTION__);
		Game::GameInstance->Bomb->changeState(Normal);
	}
	else if (name == "RemoteBomb2")
	{
		Game::GameInstance->Bomb2->setAddress(hCPU->gpr[3], __FUNCTION__);
		Game::GameInstance->Bomb2->changeState(Normal);
	}
	else if (name == "RemoteBombCube")
	{
		Game::GameInstance->BombCube->setAddress(hCPU->gpr[3], __FUNCTION__);
		Game::GameInstance->BombCube->changeState(Normal);
	}
	else if (name == "RemoteBombCube2")
	{
		Game::GameInstance->BombCube2->setAddress(hCPU->gpr[3], __FUNCTION__);
		Game::GameInstance->BombCube2->changeState(Normal);
	}

	int BombType = -1;
	
	for (int i = 0; i < 4; i++)
	{
		if (name == BombChoices[i])
			BombType = i;
	}

	if (BombType == -1)
		return;

	for (int j = 1; j < 33; j++)
	{
		if (!Instances::PlayerList[j]->connected)
			continue;

		BombAccess* Bomb = new BombAccess();

		if (BombType == 0)
			Bomb = Instances::PlayerList[j]->Bomb;
		else if (BombType == 1)
			Bomb = Instances::PlayerList[j]->Bomb2;
		else if (BombType == 2)
			Bomb = Instances::PlayerList[j]->BombCube;
		else if (BombType == 3)
			Bomb = Instances::PlayerList[j]->BombCube2;

		if (Bomb->getStatus() == Processing)
		{
			Bomb->setAddress(hCPU->gpr[3], __FUNCTION__);
			Bomb->changeState(Normal);
			std::stringstream stream;
			stream << "Assigned " << name << " to player " << std::to_string(j) << " at: " << std::hex << hCPU->gpr[3];
			Logging::LoggerService::LogInformation(stream.str(), __FUNCTION__);
			return;
		}
	}
}

void OnActorErase(PPCInterpreter_t* hCPU)
{

	hCPU->instructionPointer = hCPU->sprNew.LR;

	std::string name = Memory::read_string(Main::baseAddr + hCPU->gpr[3] + 0x10, 100, __FUNCTION__);

	std::vector<std::string> BombChoices = {"CustomRemoteBomb", "CustomRemoteBomb2", "CustomRemoteBombCube", "CustomRemoteBombCube2"};
	std::vector<std::string> RealBombChoices = { "RemoteBomb", "RemoteBomb2", "RemoteBombCube", "RemoteBombCube2" };

	if (name.rfind("Jugador", 0) == 0) {
		int spawnedPlayer = std::stoi(name.replace(0, 7, ""));
		//Instances::PlayerList[spawnedPlayer]->Delete->set(false, __FUNCTION__);
		Instances::PlayerList[spawnedPlayer]->Status->set(0, __FUNCTION__);
		Instances::PlayerList[spawnedPlayer]->setAddress(0);
		std::stringstream stream;
		stream << "Player " << spawnedPlayer << " erased.";
		Logging::LoggerService::LogDebug(stream.str(), __FUNCTION__);
	}

	if (name.rfind("Enemy_", 0) == 0)
		Game::GameInstance->EnemyService->UpdateEnemyAddress(Main::baseAddr + hCPU->gpr[3], false);

	if (!started) return;

	if (name == "RemoteBomb")
	{
		Game::GameInstance->Bomb->setAddress(0, __FUNCTION__);
		Game::GameInstance->Bomb->changeState(Cancelled);
	}
	else if (name == "RemoteBomb2")
	{
		Game::GameInstance->Bomb2->setAddress(0, __FUNCTION__);
		Game::GameInstance->Bomb2->changeState(Cancelled);
	}
	else if (name == "RemoteBombCube")
	{
		Game::GameInstance->BombCube->setAddress(0, __FUNCTION__);
		Game::GameInstance->BombCube->changeState(Cancelled);
	}
	else if (name == "RemoteBombCube2")
	{
		Game::GameInstance->BombCube2->setAddress(0, __FUNCTION__);
		Game::GameInstance->BombCube2->changeState(Cancelled);
	}

	int BombType = -1;

	for (int i = 0; i < 4; i++)
	{
		if (name == BombChoices[i])
			BombType = i;
	}

	if (BombType == -1)
		return;

	for (int j = 1; j < 33; j++)
	{
		if (!Instances::PlayerList[j]->connected)
			continue;

		BombAccess* Bomb = new BombAccess();

		if (BombType == 0)
			Bomb = Instances::PlayerList[j]->Bomb;
		else if (BombType == 1)
			Bomb = Instances::PlayerList[j]->Bomb2;
		else if (BombType == 2)
			Bomb = Instances::PlayerList[j]->BombCube;
		else if (BombType == 3)
			Bomb = Instances::PlayerList[j]->BombCube2;

		if (Bomb->BaseAddr == hCPU->gpr[3])
		{
			Bomb->setAddress(0, __FUNCTION__);
			Bomb->changeState(Deallocated);
			std::stringstream stream;
			stream << "Bomb " << name << " of player " << std::to_string(j) << " despawned.";
			Logging::LoggerService::LogInformation(stream.str(), __FUNCTION__);
			return;
		}
	}

	//for (int i = 0; i < 4; i++)
	//{

	//	if (std::string(name).rfind(BombChoices[i], 0) == 0)
	//	{

	//		for (int j = 0; j < Main::BombSync->BombAvailableAddresses[i].size(); j++)
	//		{

	//			if (Main::BombSync->BombAvailableAddresses[i][j] == hCPU->gpr[4])
	//			{

	//				Main::BombSync->OtherPlayerBombsMutex.lock();
	//				Main::BombSync->BombAvailableAddresses[i].erase(Main::BombSync->BombAvailableAddresses[i].begin() + j);
	//				Main::BombSync->OtherPlayerBombsMutex.unlock();
	//				std::cout << BombChoices[i] << " cleared" << std::endl;

	//			}

	//		}

	//	}

	//	for (int i = 0; i < 4; i++)
	//	{

	//		for (int j = 0; j < RealBombChoices.size(); j++)
	//		{

	//			if (Main::Jugadores[i]->UserBombs[RealBombChoices[j]]->BaseAddress == hCPU->gpr[3])
	//			{

	//				Main::BombSync->BombExplodeMutex.lock();

	//				Main::Jugadores[i]->UserBombs[RealBombChoices[j]]->Exists = false;
	//				Main::Jugadores[i]->UserBombs[RealBombChoices[j]]->BaseAddress = 0;

	//				//std::cout << "Bomb " << RealBombChoices[j] << " for Jugador " << i + 1 << " despawned" << std::endl;

	//				Main::BombSync->BombExplodeMutex.unlock();

	//			}

	//		}

	//	}

	//}

}

void remoteBomb_onAICalc(PPCInterpreter_t* hCPU)
{
	hCPU->instructionPointer = hCPU->sprNew.LR;

	unsigned int aiPtr = hCPU->gpr[3];
	unsigned int actPtr;
	std::vector<std::string> BombChoices = { "CustomRemoteBomb", "CustomRemoteBomb2", "CustomRemoteBombCube", "CustomRemoteBombCube2" };

	memInstance->memory_readMemoryBE(aiPtr, &actPtr, Main::baseAddr);

	std::string name = Memory::read_string(Main::baseAddr + actPtr + 0x10, 50, __FUNCTION__);

	bool explode = Memory::read_bytes(aiPtr + 0x78 + Main::baseAddr, 1, __FUNCTION__)[0] == 0x1 ? true : false;
	bool cancel = Memory::read_bytes(aiPtr + 0x9C + Main::baseAddr, 1, __FUNCTION__)[0] == 0x1 ? true : false;

	if(explode || cancel)
	{
		if (name == "RemoteBomb")
		{
			Game::GameInstance->Bomb->setAddress(0, __FUNCTION__);
			Game::GameInstance->Bomb->changeState(explode ? Exploded : Cancelled); // Assuming it can only explode or cancel, not both
		}
		else if (name == "RemoteBomb2")
		{
			Game::GameInstance->Bomb2->setAddress(0, __FUNCTION__);
			Game::GameInstance->Bomb2->changeState(explode ? Exploded : Cancelled);
		}
		else if (name == "RemoteBombCube")
		{
			Game::GameInstance->BombCube->setAddress(0, __FUNCTION__);
			Game::GameInstance->BombCube->changeState(explode ? Exploded : Cancelled);
		}
		else if (name == "RemoteBombCube2")
		{
			Game::GameInstance->BombCube2->setAddress(0, __FUNCTION__);
			Game::GameInstance->BombCube2->changeState(explode ? Exploded : Cancelled);
		}
	}

	int BombType = -1;

	for (int i = 0; i < 4; i++)
	{
		if (name == BombChoices[i])
			BombType = i;
	}

	if (BombType == -1)
		return;

	for (int j = 1; j < 33; j++)
	{
		if (!Instances::PlayerList[j]->connected)
			continue;

		BombAccess* Bomb = new BombAccess();

		if (BombType == 0)
			Bomb = Instances::PlayerList[j]->Bomb;
		else if (BombType == 1)
			Bomb = Instances::PlayerList[j]->Bomb2;
		else if (BombType == 2)
			Bomb = Instances::PlayerList[j]->BombCube;
		else if (BombType == 3)
			Bomb = Instances::PlayerList[j]->BombCube2;

		if (Bomb->getStatus() == Deallocated)
			continue;

		if (Bomb->BaseAddr == actPtr)
		{
			BombStatus status = Bomb->getStatus();
			if (status == Exploded)
			{
				Memory::write_byte(aiPtr + 0x78 + Main::baseAddr, 0x1, __FUNCTION__);
				Bomb->changeState(Deallocated);
				std::stringstream stream;
				stream << "Bomb " << name << " of player " << std::to_string(j) << " exploded.";
				Logging::LoggerService::LogInformation(stream.str(), __FUNCTION__);
				Bomb->setAddress(0, __FUNCTION__);
				return;
			}
			else if (status == Cancelled)
			{
				Memory::write_byte(aiPtr + 0x9C + Main::baseAddr, 0x1, __FUNCTION__);
				Bomb->changeState(Deallocated);
				std::stringstream stream;
				stream << "Bomb " << name << " of player " << std::to_string(j) << " erased.";
				Logging::LoggerService::LogInformation(stream.str(), __FUNCTION__);
				Bomb->setAddress(0, __FUNCTION__);
				return;
			}
		}
	}
}

void remoteBomb_onAICalc2(PPCInterpreter_t* hCPU)
{

	std::vector<std::string> RealBombChoices = { "RemoteBomb", "RemoteBomb2", "RemoteBombCube", "RemoteBombCube2" };

	hCPU->instructionPointer = hCPU->sprNew.LR;
	
	unsigned int aiPtr = hCPU->gpr[3];
	unsigned int actPtr;
	memInstance->memory_readMemoryBE(aiPtr, &actPtr, Main::baseAddr);

	std::string name = Memory::read_string(Main::baseAddr + actPtr + 0x10, 50, __FUNCTION__);

	bool explode = Memory::read_bytes(aiPtr + 0x78 + Main::baseAddr, 1, __FUNCTION__)[0] == 0x1 ? true : false;
	bool cancel = Memory::read_bytes(aiPtr + 0x9C + Main::baseAddr, 1, __FUNCTION__)[0] == 0x1 ? true : false;

	Main::BombSync->BombExplodeMutex.lock();

	for (int i = 0; i < 4; i++)
	{
		if (name.rfind(RealBombChoices[i], 0) == 0)
		{

			if (explode)
			{
				Main::BombSync->BombMutex.lock();
				Main::BombSync->Bombs[name] = 0;
				Main::BombSync->BombMutex.unlock();
			}
			else if (cancel)
			{
				Main::BombSync->BombMutex.lock();
				Main::BombSync->Bombs[name] = -1;
				Main::BombSync->BombMutex.unlock();
			}
		}
	}

	Main::BombSync->BombExplodeMutex.unlock();

	Main::BombSync->BombMutex.lock();
	for (int i = 0; i < Main::BombSync->BombsToExplode.size(); i++)
		if (Main::BombSync->BombsToExplode[i] == actPtr)
		{
			Memory::write_byte(aiPtr + 0x78 + Main::baseAddr, 0x1, __FUNCTION__);
			Main::BombSync->BombsToExplode.erase(Main::BombSync->BombsToExplode.begin() + i);
		}
	
	for (int i = 0; i < Main::BombSync->BombsToClear.size(); i++)
		if (Main::BombSync->BombsToClear[i] == actPtr)
		{
			Memory::write_byte(aiPtr + 0x9C + Main::baseAddr, 0x1, __FUNCTION__);
			Main::BombSync->BombsToClear.erase(Main::BombSync->BombsToClear.begin() + i);
		}
	Main::BombSync->BombMutex.unlock();
}

void timemgr_OnInit(PPCInterpreter_t* hCPU)
{
	hCPU->instructionPointer = hCPU->sprNew.LR;

	Game::GameInstance->World->ReadableTime = new BigEndian<float>(hCPU->gpr[3] + 0x98 + Main::baseAddr, __FUNCTION__);
	Game::GameInstance->World->WritableTime = new BigEndian<float>(hCPU->gpr[3] + 0xA0 + Main::baseAddr, __FUNCTION__);

	//Game::GameInstance->World->ReadableTime->setAddress(hCPU->gpr[3] + 0x98 + Main::baseAddr, __FUNCTION__, true);
	//Game::GameInstance->World->WritableTime->setAddress(hCPU->gpr[3] + 0xA0 + Main::baseAddr, __FUNCTION__, true);

	//std::stringstream stream;
	//stream << "Time offset: " << std::hex << hCPU->gpr[3];

	//Logging::LoggerService::LogDebug(stream.str());
}

void init() {
	osLib_registerHLEFunctionType osLib_registerHLEFunction = (osLib_registerHLEFunctionType)GetProcAddress(GetModuleHandleA("Cemu.exe"), "osLib_registerHLEFunction");
	osLib_registerHLEFunction("spawnactors", "fnCallMain", static_cast<void (*) (PPCInterpreter_t*)>(&mainFn)); // Give our assembly patch something to hook into
	osLib_registerHLEFunction("multiplayer", "WeatherSync", static_cast<void (*) (PPCInterpreter_t*)>(&WeatherFn)); // Give our assembly patch something to hook into
	osLib_registerHLEFunction("ukl_actorinterceptor", "OnActorCreate", static_cast<void (*) (PPCInterpreter_t*)>(&OnActorCreate)); // Give our assembly patch something to hook into
	osLib_registerHLEFunction("ukl_actorinterceptor", "OnActorDeleteLater", static_cast<void (*) (PPCInterpreter_t*)>(&OnActorErase)); // Give our assembly patch something to hook into
	osLib_registerHLEFunction("ukl_remotebombaiinterceptor", "OnCalc", &remoteBomb_onAICalc);
	osLib_registerHLEFunction("ukl_timemgrinterceptor", "OnInit", &timemgr_OnInit);
}