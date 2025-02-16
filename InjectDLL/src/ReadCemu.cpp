#include "Memory.h"
#include <iostream>

uint64_t Memory::getBaseAddress()
{
    if (base_addr == 0)
    {
        Memory::memory_getBaseType memory_getBase = (Memory::memory_getBaseType)GetProcAddress(GetModuleHandle("Cemu.exe"), "memory_getBase");
        base_addr = (uint64_t)memory_getBase();
    }

    return base_addr;
}

DWORD Memory::read_memory(uint64_t Addr, const char* caller)
{
    return *(DWORD*)(Addr);
}

void Memory::ValidateAddress(uint64_t address)
{

    if (address < base_addr)
        throw std::invalid_argument("");

}

int Memory::swap_Endian(int number)
{
    int byte0, byte1, byte2, byte3;
    byte0 = (number & 0x000000FF) >> 0;
    byte1 = (number & 0x0000FF00) >> 8;
    byte2 = (number & 0x00FF0000) >> 16;
    byte3 = (number & 0xFF000000) >> 24;
    return ((byte0 << 24) | (byte1 << 16) | (byte2 << 8) | (byte3 << 0));
}

std::vector<BYTE> Memory::read_bytes(uint64_t Addr, int bytes, const char* caller)
{
    
    try
    {

        std::vector<BYTE> bytesRead;

        for (int i = 0; i < bytes; i++)
        {
            bytesRead.push_back(*(BYTE*)(Addr + i));
        }

        return bytesRead;
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to read bytes.", Message);

        exit(1);
    }

}

float Memory::read_bigEndianFloat(uint64_t Addr, const char* caller)
{
    try
    {
        float bigf;
        DWORD result = *(DWORD*)(Addr);
        int swapped_value = Memory::swap_Endian(int(result));
        memcpy(&bigf, &swapped_value, 4);
        return bigf;
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to read float.", Message);

        exit(1);
    }
}

int Memory::read_bigEndian4Bytes(uint64_t Addr, const char* caller)
{
    try 
    {
        return Memory::swap_Endian(int(*(DWORD*)(Addr)));
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to read 4 bytes.", Message);

        exit(1);
    }
}

int Memory::read_bigEndian4BytesOffset(uint64_t Offset, const char* caller)
{
    try
    {
        return Memory::swap_Endian(int(*(DWORD*)(Offset + getBaseAddress())));
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to read 4 bytes.", Message);

        exit(1);
    }
}

std::string Memory::read_string(uint64_t Addr, int bytes, const char* caller)
{
    try
    {

        std::vector<BYTE> bytesRead = Memory::read_bytes(Addr, bytes, __FUNCTION__);

    std::vector<BYTE>::iterator endString = std::find(bytesRead.begin(), bytesRead.end(), 0x00);

        if (endString != bytesRead.end())
        {
            bytesRead.erase(endString, bytesRead.end());
        }

        std::string stringRead(bytesRead.begin(), bytesRead.end());

        return stringRead;
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to read string.", Message);

        exit(1);
    }

}

void Memory::write_bigEndianFloat(uint64_t Addr, float value, const char* caller)
{
    try
    {

        int val_int;
        memcpy(&val_int, &value, 4);
        *(DWORD*)(Addr) = (DWORD)Memory::swap_Endian(val_int);
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to write float.", Message);

        exit(1);
    }

}

void Memory::write_bigEndian4Bytes(uint64_t Addr, int value, const char* caller)
{
    try
    {
        *(DWORD*)(Addr) = (DWORD)Memory::swap_Endian(value);
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to write 4 bytes.", Message);

        exit(1);
    }

}

void Memory::write_byte(uint64_t Addr, BYTE byte, const char* caller)
{
    try
    {
        *(BYTE*)(Addr) = byte;
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to write byte.", Message);

        exit(1);
    }

}

void Memory::write_bytes(uint64_t Addr, std::vector<BYTE> bytes, const char* caller)
{
    try
    {
        for (int i = 0; i < bytes.size(); i++)
        {
            *(BYTE*)(Addr + i) = bytes[i];
        }
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to write bytes.", Message);

        exit(1);
    }

}

void Memory::write_string(uint64_t Addr, std::string string, int bytes, const char* caller)
{
    try
    {
        if (bytes == 0)
            bytes = string.size();

        if (bytes != 0)
        {
            for (int i = 0; i < bytes; i++)
            {
                BYTE byteToWrite = 0x00;

                if (i < string.size())
                    byteToWrite = string[i];

                *(BYTE*)(Addr + i) = byteToWrite;
            }
        }
    }
    catch (...)
    {

        char Message[100];

        strcpy_s(Message, caller);
        strcat_s(Message, " -> ");
        strcat_s(Message, __FUNCTION__);

        Logging::LoggerService::LogError("Failed to write string.", Message);

        exit(1);
    }

}

std::vector<BYTE> Memory::getNop(int length)
{
    std::vector<BYTE> result;

    for (int i = 0; i < length; i++)
    {
        result.push_back(0x90);
    }

    return result;
}

std::string Memory::hexStr(std::vector<BYTE> data)
{
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < data.size(); i++)
    {
        ss << std::setw(2) << std::setfill('0') << (int)data[i];
    }

    return ss.str();
}

std::string Memory::extractLocName(uint64_t Addr, int bytes)
{
    std::string locString = Memory::read_string(Addr, bytes, __FUNCTION__);
    std::string result = "";

    for (int i = 0; i < locString.size(); i++)
    {
        if (locString[i] == ' ')
        {
            if (i == 0)
            {
                result = " ";
            }

            break;
        }

        result += locString[i];
    }

    return result;
}

bool Memory::CompareSignatures(std::vector<BYTE> First, std::vector<BYTE> Second, std::vector<int> WildCards)
{
    for (int i = 0; i < First.size(); i++)
    {
        if (std::find(WildCards.begin(), WildCards.end(), i) != WildCards.end()) continue;

        if (First[i] != Second[i]) return false;
    }

    return true;
}

uint64_t Memory::ReadPointers(uint64_t InitialAddress, std::vector<int> readingOffsets, bool IncludeBaseAddress)
{
    uint64_t response = InitialAddress;

    for (int i = 0; i < readingOffsets.size(); i++)
    {
        try 
        {
            response = read_bigEndian4BytesOffset(response + readingOffsets[i], __FUNCTION__);
            if (response == 0) return 0;
        }
        catch (...)
        {
            std::cout << "[WRN] Couldn't read pointer at position " << i << " tried to read offset " << std::hex << response + readingOffsets[i] << std::endl;
            return 0;
        }
    }

    return response + (IncludeBaseAddress ? getBaseAddress() : 0);
}