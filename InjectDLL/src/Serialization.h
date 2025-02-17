#pragma once

#include <vector>
#include <string>
#include <cstring>  // For memcpy
#include <cstdint>  // For uint8_t, short
#include "ClientDTO.h"
#include "ServerDTO.h"

namespace Serialization
{
	static short currentIndex = 0;
	static uint8_t ClientData[7168];

	static class Serializer
	{
	private:
		static void copyData(void* Dst, const void* Src, int size)
		{
			memcpy(Dst, Src, size);
			currentIndex += size;
		}

		static int copyIntFromShort(const void* Src)
		{
			short temp = 0;
			copyData(&temp, Src, 2);
			return temp;
		}

		static DTO::WorldDTO* DeserializeWorldData(std::vector<uint8_t> input);
		static DTO::NamesDTO* DeserializeNameData(std::vector<uint8_t> input);
		static DTO::ModelsDTO* DeserializeModelDTO(std::vector<uint8_t> input);
		static DTO::CloseCharacterDTO* DeserializeCloseCharacter(std::vector<uint8_t> input);
		static DTO::FarCharacterDTO* DeserializeFarCharacter(std::vector<uint8_t> input);
		static DTO::EnemyDTO* DeserializeEnemyData(std::vector<uint8_t> input);
		static DTO::QuestDTO* DeserializeQuestData(std::vector<uint8_t> input);
		static DTO::NetworkDTO* DeserializeNetworkData(std::vector<uint8_t> input);
		static DTO::DeathSwapDTO* DeserializeDeathSwapData(std::vector<uint8_t> input);
		static DTO::TeleportDTO* DeserializeTeleportData(std::vector<uint8_t> input);
		static DTO::PropHuntDTO* DeserializePropHuntData(std::vector<uint8_t> input);
		static DataTypes::ModelData* DeserializeModelData(std::vector<uint8_t> input);
		static DataTypes::BumiiData* DeserializeBumiiData(std::vector<uint8_t> input);

		static void SerializeWorldData(DTO::WorldDTO* input);
		static void SerializeCharacterData(DTO::ClientCharacterDTO* input);
		static void SerializeEnemyData(DTO::EnemyDTO* input);
		static void SerializeQuestData(DTO::QuestDTO* input);

	public:
		static DTO::ServerDTO* DeserializeServerData(uint8_t* inputBytes);
		static void SerializeConnectData(uint8_t* outputArray, std::string name, std::string password, std::string modelType, std::string modelData);
		static void SerializeDisconnectData(uint8_t* outputArray, std::string reason);
		static void SerializeClientData(uint8_t* outputArray, DTO::ClientDTO* input);

		static void CopyToArray(uint8_t* array);
		static std::string CopyString(std::vector<uint8_t> input);
	};

}
