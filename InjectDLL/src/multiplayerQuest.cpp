#pragma once

#include "Memory.h"

using namespace Memory;

std::vector<uint64_t> MultiplayerQuest::findMQuests(uint64_t offset)
{

    std::vector<uint64_t> result;

    std::vector<int> sig;

    sig = { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, -1, 0xB7, 0x3D, 0xF1, 0x97 };
    result.push_back(Memory::PatternScan(sig, Memory::getBaseAddress(), 8, offset) + 0xB);

    sig = { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, -1, 0xF6, 0x4A, 0xA6, 0x09 };
    result.push_back(Memory::PatternScan(sig, Memory::getBaseAddress(), 8, offset) + 0xB);

    sig = { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, -1, 0x50, 0x05, 0x5B, 0x6B };
    result.push_back(Memory::PatternScan(sig, Memory::getBaseAddress(), 8, offset) + 0xB);

    return result;

}

void  MultiplayerQuest::startMQuest(std::vector<uint64_t> questsAddr)
{

    /*for (int i = 0; i < questsAddr.size(); i++)
    {
        Memory::write_byte(questsAddr[i], 0x1);
    }*/

    Memory::write_byte(questsAddr[1], 0x1, __FUNCTION__);

}

void MultiplayerQuest::changeMultiplayerQuestStatus(uint64_t addr, bool status)
{
    Memory::write_byte(addr, status ? 0x01 : 0x00, __FUNCTION__);
}

void MultiplayerQuest::changeMQuestSvName(std::string serverName, uint64_t offset)
{
    std::vector<int> sig = { 0x00, 0x00, 0x00, 0x00, -1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC7, 0xF8, 0xCD, 0x27 };
    Memory::write_string(Memory::PatternScan(sig, Memory::getBaseAddress(), 8, offset), serverName, 32, __FUNCTION__);
}

void MultiplayerQuest::changeMQuestPing(int ping)
{
    Memory::write_bigEndian4Bytes(pingAddress, ping, __FUNCTION__);
}

void MultiplayerQuest::findMQuestPingAddress(uint64_t offset)
{
    std::vector<int> sig = { 0x13, 0x88, -1, -1, -1, -1, 0xF6, 0x45, 0x81, 0x63 };
    pingAddress = Memory::PatternScan(sig, Memory::getBaseAddress(), 8, offset) + 0x2;
}