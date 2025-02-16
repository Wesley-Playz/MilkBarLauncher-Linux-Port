#pragma once

#include "Memory.h"
#include "LoggerService.h"

using namespace Memory;

void MessagerService::StartMessagerService()
{

	std::vector<std::vector<int>> sigs = {
		{0x9E, 0xA8, 0xFD, 0xEC, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x86, 0x38},
		{0xA3, 0xA4, 0x58, 0x85, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84, 0x10}
	};

	MessageStringAddr = Memory::PatternScan(sigs[0], getBaseAddress(), 8, 0x40000000) - 0x20;
	MessageQuestAddr = Memory::PatternScan(sigs[1], getBaseAddress(), 8, 0x40000000) - 0x1;

}

void MessagerService::DisplayMessage()
{
	if (MessageQueue.size() == 0)
		return;

	if (LastMessageTime != -1 && (float(GetTickCount() - LastMessageTime) / 1000) < 6)
		return;

	std::string Message = MessageQueue.back();
	MessageQueue.pop_back();

	Logging::LoggerService::LogInformation("Displayed \"" + Message + "\" " + std::to_string(MessageQueue.size()) + " messages left in queue.", __FUNCTION__);

	Memory::write_string(MessageStringAddr, Message, 32, __FUNCTION__);
	Memory::write_byte(MessageQuestAddr, 0x0, __FUNCTION__);

	Sleep(1000);

	Memory::write_byte(MessageQuestAddr, 0x1, __FUNCTION__);

	Sleep(600);

	LastMessageTime = GetTickCount();
}

void MessagerService::AddMessage(std::string Message)
{
	MessageQueue.insert(MessageQueue.begin(), Message);
}

