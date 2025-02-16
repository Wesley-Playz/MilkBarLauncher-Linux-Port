#pragma once

#include "Memory.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>

using namespace Memory;

rapidjson::Document Quests_class::readQuestFlags()
{

	char* appdata = nullptr;
	size_t sz = 0;

	_dupenv_s(&appdata, &sz, "APPDATA");

	std::string str(appdata);

	std::string filepath = "\\BOTWM\\QuestFlags.txt";

	std::ifstream file(appdata + filepath);

	std::stringstream buffer;
	buffer << file.rdbuf();

	rapidjson::Document doc;
	doc.Parse(buffer.str().c_str());

	rapidjson::StringBuffer buf;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buf);

	doc.Accept(writer);

	return doc;
}

void Quests_class::scanQuestMemory(std::vector<std::string> QuestsToSync)
{

	rapidjson::Document Quests = readQuestFlags();

	std::string QuestTypes = Quests["QuestTypes"].GetString();

	int startPosition = 0;
	std::string substring;

	for (int i = 0; i < QuestTypes.size(); i++)
	{
		if (QuestTypes[i] == '|')
		{
			substring = QuestTypes.substr(startPosition, i - startPosition);

			startPosition = i + 1;

			if (std::find(QuestsToSync.begin(), QuestsToSync.end(), substring) == QuestsToSync.end()) continue;

			numberOfQuests[substring] = Quests[substring.c_str()].GetInt();
		}
	}

	Logging::LoggerService::StartTimer("Quest sync read");
	Logging::LoggerService::LogInformation("Quest flag read started", __FUNCTION__);

	std::vector<int> sig;

	sig = { 0xFE, 0x4D, 0x15, 0x01, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84, 0x10 };

	IsGetPlayerStole2Address = Memory::PatternScan(sig, getBaseAddress(), 8) - 1;

	sig = { 0xE6, 0x05, 0xCE, 0x62, 0x00, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84, 0xC8 };

	DungeonClearCounterAddress = Memory::PatternScan(sig, getBaseAddress(), 8) - 4;

	for (auto const& pair : numberOfQuests)
	{
		std::string QType = pair.first;
		int QNumber = pair.second;

		totalQuests += QNumber;

		uint64_t offset = 0;

		for (int i = 0; i < QNumber; i++)
		{
			std::string hexString = Quests[(QType + std::to_string(i)).c_str()][0].GetString();

			startPosition = 0;

			sig = { };

			for (int j = 0; j < hexString.size(); j++)
				if (hexString[j] == ' ')
				{
					substring = hexString.substr(startPosition, j - startPosition);
					startPosition = j + 1;

					sig.push_back(std::stoul(substring, nullptr, 16));
				}

			if (i == 0)
			{
				std::vector<int> sigBase = { -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84 };

				for (int j = 0; j < sigBase.size(); j++)
					sig.push_back(sigBase[j]);

				if (QType == "L")
					sig.push_back(0xC8);
				else
					sig.push_back(0x10);
			}

			Quest newQuest;

			newQuest.Type = QType;
			newQuest.Address = i == 0 ? Memory::PatternScan(sig, getBaseAddress(), 8, offset) - 0x1 : -1;
			//newQuest.Address = Memory::PatternScan(sig, getBaseAddress(), 8, offset) - 0x1;
			newQuest.Name = Quests[(QType + std::to_string(i)).c_str()][1].GetString();

			if (newQuest.Address == -1)
			{
				MEMORY_BASIC_INFORMATION mbi{ 0 };

				uint64_t startAddr = getBaseAddress();

				for (int k = 0; k < 7; k++)
					if (VirtualQuery((LPCVOID)startAddr, &mbi, sizeof(mbi)))
							startAddr += mbi.RegionSize;
				
				bool found = false;
				int offset2 = 16;

				if (QType == "L") offset2 = 32;

				while (!found)
				{
					if (offset + offset2 >= 0xE2000000)
							break;

					for (int k = 0; k < 4; k++)
					{
						byte reading = *(byte*)(startAddr + offset + offset2 + k);

						if (reading != sig[k])
						{
							if (reading == 0x10)
								if (*(byte*)(startAddr + offset + offset2 + 1) == 0x29 && *(byte*)(startAddr + offset + offset2 + 2) == 0x84)
									offset2 += 8;

							else if (reading == 0x00)
								if (*(byte*)(startAddr + offset + offset2 + 1) == 0x00 && *(byte*)(startAddr + offset + offset2 + 2) == 0x00 && *(byte*)(startAddr + offset + offset2 + 3) == 0x00)
									offset2 += 4;

							offset2 += 16;
							if (QType == "L") offset2 += 16;
							break;

							offset2 += 4;
						}
						else if (k == 3)
							found = true;
					}
				}

				if(found)
					newQuest.Address = startAddr + offset + offset2 - 0x1;
			}

			if (newQuest.Address == -1)
			{
				std::vector<int> sigBase = { -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84 };

				for (int j = 0; j < sigBase.size(); j++)
					sig.push_back(sigBase[j]);

				if (QType == "L")
					sig.push_back(0xC8);
				else
					sig.push_back(0x10);

				newQuest.Address = Memory::PatternScan(sig, getBaseAddress(), 8) - 0x1;
			}

			//newQuest.Value = read_bytes(newQuest.Address)[0];
			newQuest.Value = 0;

			MEMORY_BASIC_INFORMATION mbi{ 0 };

			uint64_t startAddr = getBaseAddress();

			for (int k = 0; k < 7; k++)
				if (VirtualQuery((LPCVOID)startAddr, &mbi, sizeof(mbi)))
					startAddr += mbi.RegionSize;

			offset = newQuest.Address - startAddr + 0x1;

			QuestList[QType + std::to_string(i)] = newQuest;
		}
	}

	Logging::LoggerService::FinishTimer();

}

void Quests_class::readQuests()
{
	for (auto const& pair : numberOfQuests)
	{
		std::string QType = pair.first;
		int QNumber = pair.second;

		for (int i = 0; i < QNumber; i++)
		{
			QuestList[QType + std::to_string(i)].updateValue();

			if (QuestList[QType + std::to_string(i)].changed)
			{
				changedQuests.push_back(QType + std::to_string(i));
				QuestList[QType + std::to_string(i)].changed = false;
			}
		}
	}
}

void Quests_class::setup(std::vector<std::string> questServerSettings, bool (*isPausedMethod)())
{
	std::cout << "Quest sync started" << std::endl;

	IsPaused = isPausedMethod;
	findBoolFlagToChange();
	findIntFlagToChange();
	findItemFlagToChange();
	findAddingAddresses(0);
	scanQuestMemory(questServerSettings);

	std::cout << "Quest sync ready" << std::endl;
}

std::vector<std::string> Quests_class::getChangedQuests()
{

	std::vector<std::string> changedCopy = changedQuests;
	std::vector<std::string> result;

	if (changedQuests.size() > 100)
	{

		for (int i = 0; i < 100; i++)
		{

			result.push_back(changedCopy[0]);
			changedQuests.erase(changedQuests.begin());
			changedCopy.erase(changedCopy.begin());

		}

	}
	else
	{

		result = changedQuests;
		changedQuests = {};

	}

	/*std::vector<std::string> changedCopy = changedQuests;
	changedQuests = {};*/

	return result;

}

DTO::QuestDTO* Quests_class::getQuestDTO()
{
	DTO::QuestDTO* result = new DTO::QuestDTO();

	result->Completed = getChangedQuests();

	return result;
}

bool Quests_class::updateQuests(bool eventStatus)
{
	bool result = false;

	while (questsToChange.size() > 0)
	{
		result = true;

		if (!eventStatus)
			return result;

		if (QuestList.count(questsToChange[0]))
		{
			//BYTE valueToWrite = QuestList[questsToChange[0]].Type == "K" ? 0x17 : 0x1;

			BYTE valueToWrite = 0x1;

			if (QuestList[questsToChange[0]].Type == "K")
				valueToWrite = 0x17;
			else if (QuestList[questsToChange[0]].Type == "C")
				if (QuestList[questsToChange[0]].Name.find("Clear_Dungeon") != std::string::npos)
					valueToWrite = 0x03;

			if (QuestList[questsToChange[0]].Type == "L")
			{
				if (QuestList[questsToChange[0]].Value == 0x0)
				{
					if (std::count(intsToChange.begin(), intsToChange.end(), QuestList[questsToChange[0]].Name))
						continue;

					intsToChange.push_back(QuestList[questsToChange[0]].Name);
					QuestList[questsToChange[0]].Value = valueToWrite;
					QuestList[questsToChange[0]].beingChanged = true;
					serverQuests.push_back(questsToChange[0]);
				}
			}
			else
			{
				if (QuestList[questsToChange[0]].Value != valueToWrite)
				{
					if (std::count(boolsToChange.begin(), boolsToChange.end(), QuestList[questsToChange[0]].Name))
						continue;

					boolsToChange.push_back(QuestList[questsToChange[0]].Name);
					QuestList[questsToChange[0]].Value = valueToWrite;
					QuestList[questsToChange[0]].beingChanged = true;
					serverQuests.push_back(questsToChange[0]);

					if (QuestList[questsToChange[0]].Type == "K") koroksToAdd++;
				}
			}
		}

		questsToChange.erase(questsToChange.begin());
	}

	return result;
}

void Quests_class::findAddingAddresses(uint64_t offset)
{
	std::vector<int> sig = { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, -1, -1, 0x01, 0x01, 0x01, 0x94, 0xC2, 0xF7 };
	addingKorokAddress = Memory::TryPatternScan(sig, Memory::getBaseAddress(), 8, offset, false, false, 0, 0, "Adding korok") + 0xB;

	sig = { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, -1, -1, 0x01, 0x01, 0x5B, 0xB4, 0xF3, 0x20 };
	addingBoolAddress = Memory::TryPatternScan(sig, Memory::getBaseAddress(), 8, offset, false, false, 0, 0, "Adding bool") + 0xB;

	sig = { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, -1, -1, 0x01, 0x01, 0xFA, 0x1C, 0x5A, 0xB9 };
	addingIntAddress = Memory::TryPatternScan(sig, Memory::getBaseAddress(), 8, offset, false, false, 0, 0, "Adding int") + 0xB;

	sig = { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, -1, -1, 0x01, 0x01, 0x11, 0x2E, 0xE0, 0xAC };
	addingItemAddress = Memory::TryPatternScan(sig, Memory::getBaseAddress(), 8, offset, false, false, 0, 0, "Adding item") + 0xB;
}

void Quests_class::findBoolFlagToChange()
{
	std::vector<int> sig = { 0x47, 0x62, 0x6F, 0x6F, 0x6C, 0x65, 0x61, -1, 0x46, 0x6C, 0x61, 0x67, 0x54, 0x6F, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65 };
	boolFlagAddress = Memory::TryPatternScan(sig, Memory::getBaseAddress(), 0, 0, false, true, 0, 0, "Bool flag") + 0x1;
}

void Quests_class::findIntFlagToChange()
{
	std::vector<int> sig = { 0x47, 0x69, 0x6E, 0x74, 0x65, 0x67, 0x65, -1, 0x46, 0x6C, 0x61, 0x67, 0x54, 0x6F, 0x43, 0x68, 0x61, 0x6E, 0x67, 0x65 };
	intFlagAddress = Memory::TryPatternScan(sig, Memory::getBaseAddress(), 0, 0, false, true, 0, 0, "Int flag") + 0x1;
}

void Quests_class::findItemFlagToChange()
{
	std::vector<int> sig = { 0x29, 0x45, 0x76, 0x65, 0x6E, 0x74, 0x49, 0x74, 0x65, 0x6D, 0x54, 0x6F, 0x43, 0x68, 0x61, 0x6E, 0x67, -1, 0x4D, 0x61, 0x6B, 0x69, 0x6E, 0x67, 0x49, 0x74, 0x4C, 0x6F, 0x6E, 0x67, 0x65, 0x72, 0x4A, 0x75, 0x73, 0x74, 0x49, 0x6E, 0x43, 0x61, 0x73, 0x65 };
	itemFlagAddress = Memory::TryPatternScan(sig, Memory::getBaseAddress(), 0, 0, false, true, 0, 0, "Item flag") + 0x1;
}

void Quests_class::changeFlag()
{
	int DungeonSeals = 0;
	std::vector<std::string> ToDeactivate;
	std::vector<std::string> ParagliderQuests;
	if (findQuest("FindDungeon_Ready") == "" || findQuest("FindDungeon_Activated") == "" || findQuest("FindDungeon_AllClear") == "" || findQuest("FindDungeon_1stClear") == "")
	{
		ParagliderQuests.push_back(findQuest("FindDungeon_Ready"));
		ParagliderQuests.push_back(findQuest("FindDungeon_Activated"));
		ParagliderQuests.push_back(findQuest("FindDungeon_AllClear"));
		ParagliderQuests.push_back(findQuest("FindDungeon_1stClear"));
	}

	Logging::LoggerService::LogInformation("Started flag change service", __FUNCTION__);

	while (true)
	{
		bool resyncParaglider = false;

		for (int i = 0; i < ParagliderQuests.size(); i++)
			if (QuestList[ParagliderQuests[i]].Value != 1)
				resyncParaglider = true;

		if (resyncParaglider)
			if (Memory::read_bigEndian4Bytes(DungeonClearCounterAddress, __FUNCTION__) > 3)
			{
				boolsToChange.push_back("FindDungeon_Ready");
				boolsToChange.push_back("FindDungeon_Activated");
				boolsToChange.push_back("FindDungeon_AllClear");
				boolsToChange.push_back("FindDungeon_1stClear");
				std::cout << "Resynced paraglider" << std::endl;
			}

		if (addingKorokAddress < 30000 || addingBoolAddress < 30000 || addingIntAddress < 30000 || addingItemAddress < 30000)
		{
			Logging::LoggerService::LogError("Failed to find a quest sync flag.");
			exit(1);
		}
		
		while (Memory::read_bytes(addingKorokAddress, 1, __FUNCTION__)[0] == 0x00 || Memory::read_bytes(addingBoolAddress, 1, __FUNCTION__)[0] == 0x00 || Memory::read_bytes(addingIntAddress, 1, __FUNCTION__)[0] == 0x00 || Memory::read_bytes(addingItemAddress, 1, __FUNCTION__)[0] == 0x00) {}

		for (int i = 0; i < ToDeactivate.size(); i++)
			QuestList[ToDeactivate[i]].beingChanged = false;

		ToDeactivate.clear();

		if (koroksToAdd == 0 && boolsToChange.size() == 0 && intsToChange.size() == 0)
		{
			Sleep(50);
			continue;
		}

		if (IsPaused())
		{
			Logging::LoggerService::LogDebug("Quest sync paused as the game is paused");
			Sleep(1000);
			continue;
		}

		if (koroksToAdd > 0)
		{
			Logging::LoggerService::LogDebug("Added korok", __FUNCTION__);
			Memory::write_byte(addingKorokAddress, 0x00, __FUNCTION__);
			koroksToAdd--;
		}

		if (boolsToChange.size() > 0)
		{
			std::string ID;
			Quest quest;

			for (auto const& pair : QuestList)
			{
				ID = pair.first;
				quest = pair.second;

				if (boolsToChange[0] == quest.Name)
					break;
			}

			if (boolsToChange[0].find("Clear_Dungeon") != std::string::npos)
			{
				itemsToAdd.push_back("Obj_DungeonClearSeal");
				intsToChange.push_back("DungeonClearCounter");
			}

			if (boolsToChange[0].find("FindDungeon_Finish") != std::string::npos)
			{
				boolsToChange.push_back("Find_Impa_Activated");
				boolsToChange.push_back("GanonQuest_Activated");
				boolsToChange.push_back("Npc_King001_Appear");
				boolsToChange.push_back("Npc_King001_Disappear");

				if (Memory::read_bytes(IsGetPlayerStole2Address, 1, __FUNCTION__)[0] == 0)
					itemsToAdd.push_back("PlayerStole2");
			}

			Memory::write_string(boolFlagAddress, boolsToChange[0], 0x47, __FUNCTION__);
			Memory::write_byte(addingBoolAddress, 0x00, __FUNCTION__);

			//QuestList[boolsToChange[0]].beingChanged = false;

			//QuestList[ID].beingChanged = false;

			ToDeactivate.push_back(ID);

			boolsToChange.erase(boolsToChange.begin());
		}

		if (intsToChange.size() > 0)
		{
			Memory::write_string(intFlagAddress, intsToChange[0], 0x47, __FUNCTION__);
			Memory::write_byte(addingIntAddress, 0x00, __FUNCTION__);

			//QuestList[intsToChange[0]].beingChanged = false;
			for (auto const& pair : QuestList)
			{

				std::string ID = pair.first;
				Quest quest = pair.second;

				if (intsToChange[0] == quest.Name)
				{
					//QuestList[ID].beingChanged = false;
					ToDeactivate.push_back(ID);
					break;
				}

			}

			intsToChange.erase(intsToChange.begin());
		}

		if (itemsToAdd.size() > 0)
		{
			Memory::write_string(itemFlagAddress, itemsToAdd[0], 0x41, __FUNCTION__);
			Memory::write_byte(addingItemAddress, 0x00, __FUNCTION__);
			itemsToAdd.erase(itemsToAdd.begin());
		}
	}
}

std::string Quests_class::findQuest(std::string QuestName)
{
	for (auto const& pair : QuestList)
	{

		std::string ID = pair.first;
		Quest quest = pair.second;

		if (QuestName == quest.Name)
		{
			return ID;
		}

	}

	return "";
}

void Quests_class::resyncQuests()
{
	for (int i = 0; i < serverQuests.size(); i++)
	{

		if (QuestList.count(serverQuests[i]))
		{

			if (QuestList[serverQuests[i]].beingChanged) continue;

			//BYTE valueToWrite = QuestList[questsToChange[0]].Type == "K" ? 0x17 : 0x1;

			BYTE valueToWrite = 0x1;

			if (QuestList[serverQuests[i]].Type == "K")
			{
				valueToWrite = 0x17;
			}
			else if (QuestList[serverQuests[i]].Type == "C")
			{
				if (QuestList[serverQuests[i]].Name.find("Clear_Dungeon") != std::string::npos)
				{
					valueToWrite = 0x03;
				}
			}

			if (QuestList[serverQuests[i]].Type == "L")
			{

				if (QuestList[serverQuests[i]].Value == 0x0)
				{

					if (std::count(intsToChange.begin(), intsToChange.end(), QuestList[serverQuests[i]].Name))
					{
						continue;
					}

					intsToChange.push_back(QuestList[serverQuests[i]].Name);
					QuestList[serverQuests[i]].Value = valueToWrite;
					QuestList[serverQuests[i]].beingChanged = true;

				}

			}
			else
			{

				if (QuestList[serverQuests[i]].Value != valueToWrite)
				{

					if (std::count(boolsToChange.begin(), boolsToChange.end(), QuestList[serverQuests[i]].Name))
					{
						continue;
					}

					boolsToChange.push_back(QuestList[serverQuests[i]].Name);
					QuestList[serverQuests[i]].Value = valueToWrite;
					QuestList[serverQuests[i]].beingChanged = true;

					if (QuestList[serverQuests[i]].Type == "K") koroksToAdd++;

				}

			}

		}

	}

}