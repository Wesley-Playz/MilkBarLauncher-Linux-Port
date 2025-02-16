#pragma once
#include "CharacterEquipment.h"
#include "Memory.h"

namespace DataTypes
{
	class EquipmentAccess
	{
	public:
		bool Changed = false;
		bool SetupFailed = false;
		CharacterEquipment* LastKnown = new CharacterEquipment();
		std::string lastBase = "Jugador1ModelNameLongForASpecificReasonHead";

		bool Compare(CharacterEquipment newEquipment)
		{
			Mutex.lock();

			if (LastKnown->WType != newEquipment.WType)
			{
				Changed = true;
				LastKnown->WType = newEquipment.WType;
			}
			if (LastKnown->Sword != newEquipment.Sword)
			{
				Changed = true;
				LastKnown->Sword = newEquipment.Sword;
			}
			if (LastKnown->Shield != newEquipment.Shield)
			{
				Changed = true;
				LastKnown->Shield = newEquipment.Shield;
			}
			if (LastKnown->Bow != newEquipment.Bow)
			{
				Changed = true;
				LastKnown->Bow = newEquipment.Bow;
			}
			if (LastKnown->Head != newEquipment.Head)
			{
				Changed = true;
				LastKnown->Head = newEquipment.Head;
			}
			if (LastKnown->Upper != newEquipment.Upper)
			{
				Changed = true;
				LastKnown->Upper = newEquipment.Upper;
			}
			if (LastKnown->Lower != newEquipment.Lower)
			{
				Changed = true;
				LastKnown->Lower = newEquipment.Lower;
			}

			Mutex.unlock();

			return Changed;
		}

		void SetWeapons(uint64_t baseAddr)
		{
			std::string RIGHT_DEFAULT = "RightHandWeaponLongName";
			std::string LEFT_DEFAULT = "LeftHandWeaponLongName";

			FindWeaponAddr(baseAddr);

			uint64_t OldAddress = ArmorAddrs.Face;

			FindArmorAddr(baseAddr);

			std::string RightHandWeapon = "Weapon_Null_";
			std::string LeftHandWeapon = "Weapon_Shield_";

			switch (LastKnown->WType)
			{
			case 1:
				RightHandWeapon.replace(RightHandWeapon.find("Null"), 4, "Sword");
				break;
			case 2:
				RightHandWeapon.replace(RightHandWeapon.find("Null"), 4, "Lsword");
				break;
			case 3:
				RightHandWeapon.replace(RightHandWeapon.find("Null"), 4, "Spear");
				break;
			}

			Mutex.lock();

			Memory::write_string(WeaponAddrs.Right, RightHandWeapon + NumToStr(LastKnown->Sword), RIGHT_DEFAULT.size() + 8, __FUNCTION__);
			Memory::write_string(WeaponAddrs.Left, LeftHandWeapon + NumToStr(LastKnown->Shield), LEFT_DEFAULT.size() + 8, __FUNCTION__);

			if (OldAddress == ArmorAddrs.Face)
				Changed = false;

			Mutex.unlock();
		}

		void SetArmor()
		{
			std::string BASE_FOLDER = "Jugador1ModelNameLongForASpecificReason";
			std::string BASE_DEFAULT = "Jugador1ModelNameLongForASpecificReasonHead";
			std::string CHEST_DEFAULT = "Jugador1ModelNameLongForASpecificReasonChest";
			std::string UPPER_DEFAULT = "Jugador1ModelNameLongForASpecificReasonHelmet";
			std::string LOWER_DEFAULT = "Jugador1ModelNameLongForASpecificReasonLower";
			std::string HEAD_DEFAULT = "Jugador1ModelNameLongForASpecificReasonUpper";

			std::string UPPER_DEFAULT_ARMOR = "MP_Armor_Default_Upper";
			std::string LOWER_DEFAULT_ARMOR = "MP_Armor_Default_Lower";
			std::string HEAD_DEFAULT_ARMOR = "MP_Armor_Default_Head";

			std::string BaseToWrite = BASE_DEFAULT;
			std::string UpperToWrite = LastKnown->Upper == 0 ? UPPER_DEFAULT_ARMOR : "MP_Armor_" + NumToStr(LastKnown->Upper) + "_Upper";
			std::string LowerToWrite = LastKnown->Lower == 0 ? LOWER_DEFAULT_ARMOR : "MP_Armor_" + NumToStr(LastKnown->Lower) + "_Lower";
			std::string HeadToWrite = LastKnown->Head == 0 ? HEAD_DEFAULT_ARMOR : "MP_Armor_" + NumToStr(LastKnown->Head) + "_Head";
			std::string ChestToWrite = CHEST_DEFAULT;

			if (UpperToWrite != "" && UpperToWrite != UPPER_DEFAULT_ARMOR)
				ChestToWrite = "EmptyModel";

			if (HeadToWrite == "MP_Armor_180_Head" || HeadToWrite == "MP_Armor_160_Head" || HeadToWrite == "MP_Armor_171_Head")
				BaseToWrite = "EmptyModel";

			if(ArmorAddrs.Face != 0)
			{
				try {
					std::string HeadStr = Memory::read_string(ArmorAddrs.Face, BASE_DEFAULT.size() + 2, __FUNCTION__);
					if (HeadStr != lastBase && HeadStr != "EmptyModel")
					{
						ArmorAddrs.Face = 0;
						Logging::LoggerService::LogInformation("Failed to set up armor. Trying again...", __FUNCTION__);
						this->SetupFailed = true;
						return;
					}

					Memory::write_string(ArmorAddrs.Folder, BASE_FOLDER, BASE_FOLDER.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Face, BaseToWrite, BASE_DEFAULT.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Chest, ChestToWrite, CHEST_DEFAULT.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Head, HeadToWrite, HEAD_DEFAULT.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Lower, LowerToWrite, LOWER_DEFAULT.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Upper, UpperToWrite, UPPER_DEFAULT.size() + 2, __FUNCTION__);

					lastBase = BaseToWrite;

					this->SetupFailed = false;
					Logging::LoggerService::LogInformation("Armor setup.", __FUNCTION__);
				}
				catch (...)
				{
					this->SetupFailed = true;
					Logging::LoggerService::LogInformation("Failed to setup armor.", __FUNCTION__);
				}
			}
			else 
			{
				Logging::LoggerService::LogInformation("Couldn't find armor address.", __FUNCTION__);
				this->SetupFailed = true;
			}
		}

		void SetModel(std::string model)
		{
			//model = "Npc_Zora_Prince:Npc_Zora_Prince";
			std::stringstream stream(model);
			std::string segment;
			std::vector<std::string> splitModel;

			while (std::getline(stream, segment, ':'))
			{
				splitModel.push_back(segment);
			}

			std::string BASE_FOLDER = "Jugador1ModelNameLongForASpecificReason";
			std::string BASE_DEFAULT = "Jugador1ModelNameLongForASpecificReasonHead";
			std::string CHEST_DEFAULT = "Jugador1ModelNameLongForASpecificReasonChest";
			std::string UPPER_DEFAULT = "Jugador1ModelNameLongForASpecificReasonHelmet";
			std::string LOWER_DEFAULT = "Jugador1ModelNameLongForASpecificReasonLower";
			std::string HEAD_DEFAULT = "Jugador1ModelNameLongForASpecificReasonUpper";

			if (ArmorAddrs.Face != 0)
			{
				try {
					Memory::write_string(ArmorAddrs.Folder, splitModel[0], BASE_FOLDER.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Face, splitModel[1], BASE_DEFAULT.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Chest, splitModel[1], CHEST_DEFAULT.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Head, splitModel[1], HEAD_DEFAULT.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Lower, splitModel[1], LOWER_DEFAULT.size() + 2, __FUNCTION__);
					Memory::write_string(ArmorAddrs.Upper, splitModel[1], UPPER_DEFAULT.size() + 2, __FUNCTION__);

					lastBase = splitModel[1];

					this->SetupFailed = false;
					Logging::LoggerService::LogInformation("Model setup.", __FUNCTION__);
				}
				catch (...)
				{
					Logging::LoggerService::LogInformation("Failed to setup model.", __FUNCTION__);
					this->SetupFailed = true;
				}
			}
			else
			{
				Logging::LoggerService::LogInformation("Could not find model address. Trying again...", __FUNCTION__);
				this->SetupFailed = true;
			}
		}

		std::string NumToStr(int number)
		{
			std::string result = std::to_string(number);

			int numberOfZeros = 3 - result.size();

			for (int i = 0; i < numberOfZeros; i++)
				result = "0" + result;

			return result;
		}

		void FindWeaponAddr(uint64_t baseAddr)
		{
			uint64_t addr = Memory::ReadPointers(baseAddr, { 0x39C, 0x78, 0x244 }, false);

			bool Found = false;
			int iterator = 0;

			while (!Found)
			{
				uint64_t temp = Memory::read_bigEndian4BytesOffset(addr + (iterator * 0x4), __FUNCTION__);

				if (int(3518303375) == Memory::read_bigEndian4BytesOffset(temp + 0x10, __FUNCTION__))
				{
					Found = true;
					addr = Memory::read_bigEndian4BytesOffset(temp + 0x4, __FUNCTION__);
					this->WeaponAddrs.Right = Memory::read_bigEndian4BytesOffset(addr + 0x1C, __FUNCTION__) + Memory::getBaseAddress();
					this->WeaponAddrs.Left = Memory::read_bigEndian4BytesOffset(addr + 0xA0, __FUNCTION__) + Memory::getBaseAddress();
				}

				iterator++;
			}
		}

		void FindArmorAddr(uint64_t baseAddr)
		{

			if (baseAddr == 0)
			{
				return;
			}

			uint64_t addr = Memory::ReadPointers(baseAddr, { 0x39C, 0x6C });

			this->ArmorAddrs.Folder = Memory::ReadPointers(addr, { 0x48C, 0x1C }, true) + 0xC;

			addr = Memory::ReadPointers(addr, { 0x4B0, 0x38 });
			this->ArmorAddrs.Face = Memory::ReadPointers(addr, { 0x0, 0xC }, true);
			addr = Memory::read_bigEndian4BytesOffset(addr + 0x10, __FUNCTION__);
			this->ArmorAddrs.Chest = Memory::ReadPointers(addr, { 0x0, 0xC }, true);
			addr = Memory::read_bigEndian4BytesOffset(addr + 0x10, __FUNCTION__);
			this->ArmorAddrs.Head = Memory::ReadPointers(addr, { 0x0, 0xC }, true);
			addr = Memory::read_bigEndian4BytesOffset(addr + 0x10, __FUNCTION__);
			this->ArmorAddrs.Lower = Memory::ReadPointers(addr, { 0x0, 0xC }, true);
			addr = Memory::read_bigEndian4BytesOffset(addr + 0x10, __FUNCTION__);
			this->ArmorAddrs.Upper = Memory::ReadPointers(addr, { 0x0, 0xC }, true);

			this->Changed = true;
		}

	private:
		std::shared_mutex Mutex;

		struct WeaponAddr 
		{
			uint64_t Left = 0;
			uint64_t Right = 0;
		} WeaponAddrs;

		struct ArmorAddr
		{
			uint64_t Folder = 0;
			uint64_t Face = 0;
			uint64_t Chest = 0;
			uint64_t Head = 0;
			uint64_t Upper = 0;
			uint64_t Lower = 0;
		} ArmorAddrs;

	};
}