#pragma once

#include <sys/mman.h> // For memory management on Linux
#include <unistd.h>   // For usleep
#include <fcntl.h>    // For file operations
#include <iostream>   // For logging
#include <vector>
#include <stdexcept>  // For exceptions
#include <fstream>    // For reading /proc/self/maps

namespace DataTypes
{
    class EquippedItems
    {
    public:
        EquippedItems()
        {
            uint64_t porchAddress = 0;
            uint64_t equippedAddress = 0;

            std::vector<int> signature = { 0x5F, 0x28, 0x32, 0x89, -1, -1, -1, -1, 0x5F, 0x28, 0x32, 0x89 };

            uint64_t addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, Memory::ScanOffset) + 0x4;

            while (addr < 30000)
            {
                usleep(2000000); // Sleep for 2 seconds (Linux uses microseconds)
                std::cerr << "Failed to find porch: " << __FUNCTION__ << std::endl;
                addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, Memory::ScanOffset) + 0x4;
            }

            porchAddress = addr;

            Memory::ScanOffset = porchAddress - Memory::RegionStart;

            signature = { 0x82, 0x48, 0x92, 0xBE, -1, -1, -1, -1, 0x82, 0x48, 0x92, 0xBE };

            addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, Memory::ScanOffset) + 0x4;

            while (addr < 30000)
            {
                usleep(2000000); // Sleep for 2 seconds (Linux uses microseconds)
                std::cerr << "Failed to find equipped items: " << __FUNCTION__ << std::endl;
                addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, Memory::ScanOffset) + 0x4;
            }

            equippedAddress = addr;

            Memory::ScanOffset = equippedAddress - Memory::RegionStart;

            setAddress(porchAddress, equippedAddress, __FUNCTION__);
        }

        void setAddress(uint64_t porchAddress, uint64_t equippedAddress, const char* caller, bool Validate = true)
        {
            if (Validate)
            {
                AddressSet = ValidateAddress(porchAddress, caller);

                if (!AddressSet)
                    return;

                AddressSet = ValidateAddress(equippedAddress, caller);

                if (!AddressSet)
                    return;
            }
            else
                AddressSet = true;

            this->PorchAddress = porchAddress;
            this->EquippedAddress = equippedAddress;
        }

        CharacterEquipment getEquipment(const char* caller)
        {
            CharacterEquipment* Equipment = new CharacterEquipment();

            Equipment->WType = 0;

            std::vector<std::string> AllEquipment = getAll(caller);

            for (int i = 0; i < AllEquipment.size(); i++)
            {
                if (AllEquipment[i].rfind("Weapon") != std::string::npos)
                {
                    int WeaponN = std::stoi(AllEquipment[i].substr(AllEquipment[i].size() - 3));

                    if (AllEquipment[i].rfind("Shield") != std::string::npos)
                        Equipment->Shield = WeaponN;
                    else if (AllEquipment[i].rfind("Bow") != std::string::npos)
                        Equipment->Bow = WeaponN;
                    else
                    {
                        if (AllEquipment[i].rfind("Sword") != std::string::npos)
                            Equipment->WType = 1;
                        else if (AllEquipment[i].rfind("Lsword") != std::string::npos)
                            Equipment->WType = 2;
                        else if (AllEquipment[i].rfind("Spear") != std::string::npos)
                            Equipment->WType = 3;

                        Equipment->Sword = WeaponN;
                    }

                    continue;
                }

                if (AllEquipment[i].rfind("Armor") != std::string::npos)
                {
                    try
                    {
                        int WeaponN = std::stoi(AllEquipment[i].substr(6, 3));

                        if (AllEquipment[i].rfind("Upper") != std::string::npos)
                            Equipment->Upper = WeaponN;
                        else if (AllEquipment[i].rfind("Lower") != std::string::npos)
                            Equipment->Lower = WeaponN;
                        else if (AllEquipment[i].rfind("Head") != std::string::npos)
                            Equipment->Head = WeaponN;
                    }
                    catch (...)
                    {
                        if (AllEquipment[i].rfind("Upper") != std::string::npos)
                            Equipment->Upper = 0;
                        else if (AllEquipment[i].rfind("Lower") != std::string::npos)
                            Equipment->Lower = 0;
                        else if (AllEquipment[i].rfind("Head") != std::string::npos)
                            Equipment->Head = 0;
                    }

                    continue;
                }
            }

            return *Equipment;
        }

        std::vector<std::string> getAll(const char* caller)
        {
            std::vector<std::string> Equipment;

            if (!AddressSet)
            {
                std::cerr << "Tried to get value of not set address. Returning default: " << caller << std::endl;
                return Equipment;
            }

            for (int i = 0; i < 420; i++)
            {
                int isEquipped = Memory::read_bigEndian4Bytes(EquippedAddress + (i * 8), __FUNCTION__);

                if (isEquipped == 1)
                {
                    std::string equipmentName = "";

                    for (int j = 0; j < 16; j++)
                        equipmentName += Memory::read_string(PorchAddress + (i * 128) + (j * 8), 4, __FUNCTION__);

                    Equipment.push_back(equipmentName);
                }
            }

            return Equipment;
        }

    private:
        uint64_t PorchAddress;
        uint64_t EquippedAddress;
        bool AddressSet = false;

        bool ValidateAddress(uint64_t Address, const char* caller)
        {
            if (Address == 0)
            {
                std::cerr << "Address set to 0: " << caller << std::endl;
                return false;
            }

            // Use mincore to check if the memory is valid
            unsigned char vec;
            if (mincore(reinterpret_cast<void*>(Address), sizeof(uint64_t), &vec) == -1)
            {
                std::cerr << "Failed to validate address: " << caller << std::endl;
                return false;
            }

            return true;
        }
    };
}