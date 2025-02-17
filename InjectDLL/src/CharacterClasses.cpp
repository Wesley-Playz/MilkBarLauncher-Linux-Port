#include "Memory.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <cstring>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <chrono>
#include <thread>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace Memory;

////////////////// Link's class //////////////////

void Link_class::UpdatePosition() {
    for (int i = 0; i < 3; i++) {
        Position[i] = Memory::read_bigEndianFloat(PosAddr + (i * 0x4), __FUNCTION__);
    }
}

void Link_class::UpdateRotation() {
    Rotation = Memory::read_bigEndianFloat(RotAddr, __FUNCTION__);
}

void Link_class::UpdateSpeed() {
    for (int i = 0; i < 3; i++) {
        Speed[i] = Memory::read_bigEndianFloat(SpdAddr + (i * 0x4), __FUNCTION__);
    }
}

void Link_class::UpdateAnimation() {
    Animation = Memory::read_bigEndian4Bytes(AnimAddr, __FUNCTION__);
}

void Link_class::UpdateLocation() {
    Map = Memory::extractLocName(locationAddr + 0x14);
    Section = Memory::extractLocName(locationAddr + 0x40);
    Town = Memory::extractLocName(townAddr + 0x20);
}

int Link_class::UpdateHealth() {
    int tempHealth = Memory::read_bigEndian4Bytes(HealthAddr, __FUNCTION__);
    int ReturnValue = 0;

    if (tempHealth != Health) {
        if (Health < tempHealth && Health == 0) {
            ReturnValue = 1;
        }
        Health = tempHealth;
        lastHealthUpdate = GetTickCount();
    }

    return ReturnValue;
}

std::vector<uint64_t> Link_class::ScanData() {
    std::vector<uint64_t> result;
    std::stringstream stream;

    WeaponDamages = readWeaponDamages();
    lastHealthReduce = GetTickCount();

    std::vector<int> sig;

    while (true) {
        sig = { 0x03, -1, 0xB5, 0xCC, 0x00, 0x00, 0x00, 0x00, 0x10, 0x54, 0xF6, 0x44, 0x10, 0x2B, 0x7E, 0x78, 0x00, 0x00, 0x00 };
        locationAddr = Memory::PatternScan(sig, getBaseAddress(), 8, 0xe000000);

        if (locationAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find location.", __FUNCTION__);
            std::this_thread::sleep_for(std::chrono::seconds(2));
            continue;
        }

        stream << "Location: " << std::hex << locationAddr << std::endl;
        break;
    }

    uint64_t startAddr = getBaseAddress();
    uint64_t offset = locationAddr - startAddr - 0x500000;

    while (true) {
        sig = { 0xff, 0xff, 0xff, 0xf1, 0x3f, -1, -1, -1, 0x80, 0x00, -1, -1, 0x11, -1, -1, -1, 0x10, 0xdf, -1, -1, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80 };
        std::vector<uint64_t> ListAddrs = Memory::PatternScanMultiple(sig, getBaseAddress(), 8, offset, false, 0, 5);

        if (ListAddrs.size() == 0) {
            continue;
        } else if (ListAddrs.size() == 1) {
            if (PosAddr < 30000) {
                PosAddr = ListAddrs[0] + 0xD4;
                stream << "Position: " << std::hex << PosAddr << std::endl;
                break;
            }
        } else {
            for (int i = 0; i < ListAddrs.size(); i++) {
                float sum = 0;
                for (int j = 0; j < 3; j++) {
                    sum += Memory::read_bigEndianFloat(ListAddrs[i] + 0xD4 + (j * 4), __FUNCTION__);
                }
                if (sum == 0) {
                    continue;
                } else {
                    PosAddr = ListAddrs[i] + 0xD4;
                    std::cout << "Found " << ListAddrs.size() << " addresses for Position" << std::endl;
                    break;
                }
            }
        }

        if (PosAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find Position.", __FUNCTION__);
            continue;
        }

        stream << "Position: " << std::hex << PosAddr << std::endl;
        break;
    }

    offset += PosAddr - locationAddr;

    while (true) {
        sig = { 0x5F, 0x28, 0x32, 0x89, -1, -1, -1, -1, 0x5F, 0x28, 0x32, 0x89 };
        porchAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000) + 0x4;

        if (porchAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find porch.", __FUNCTION__);
            continue;
        }

        stream << "Porch: " << std::hex << porchAddr << std::endl;
        break;
    }

    offset += porchAddr - PosAddr;

    while (true) {
        sig = { 0x82, 0x48, 0x92, 0xBE, -1, -1, -1, -1, 0x82, 0x48, 0x92, 0xBE };
        equippedAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000) + 0x4;

        if (equippedAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find inventory status.", __FUNCTION__);
            continue;
        }

        stream << "Equipped: " << std::hex << equippedAddr << std::endl;
        break;
    }

    offset += equippedAddr - porchAddr;

    uint64_t dayAddr;

    while (true) {
        sig = { 0x7B, 0x4B, 0xE8, 0x51, 0x00, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84, 0xC8 };
        dayAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000) - 0x04;

        if (dayAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find day.", __FUNCTION__);
            continue;
        }

        stream << "Day: " << std::hex << dayAddr << std::endl;
        break;
    }

    offset += porchAddr - dayAddr;

    while (true) {
        sig = { 0x10, 0x21, 0x2C, 0xD4, -1, -1, -1, 0xE4, -1, -1, -1, 0x98, -1, -1, -1, 0x24, 0x10, 0x21, 0x2C, 0x94, -1, -1, -1, 0xF4, 0x10, 0x21, 0x29, 0xEC, 0x00, 0x00, 0x01, 0x00 };
        townAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000);

        if (townAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find town.", __FUNCTION__);
            continue;
        }

        stream << "Town: " << std::hex << townAddr << std::endl;
        break;
    }

    offset += townAddr - equippedAddr;

    while (true) {
        sig = { 0x6E, -1, -1, 0x38, 0x00, 0x00, 0x00, -1, 0x6E, -1, -1, -1, 0x6E, -1, -1, 0x58, -1, -1, -1, -1, 0x00, 0x00, 0x00, -1, 0x6E, -1, -1, -1, 0x6E, 0xD1 };
        HealthAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000) + 0x4;

        if (HealthAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find health.", __FUNCTION__);
            continue;
        }

        stream << "Health: " << std::hex << HealthAddr << std::endl;
        break;
    }

    offset += HealthAddr - townAddr;

    while (true) {
        sig = { 0xBF, 0x80, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, -1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, -1, 0x80, 0x00, 0x00 };
        RotAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000) + 0x2C;

        if (RotAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find rotation.", __FUNCTION__);
            continue;
        }

        stream << "Rotation: " << std::hex << RotAddr << std::endl;
        break;
    }

    offset += RotAddr - HealthAddr;

    while (true) {
        sig = { 0x6E, -1, -1, 0x68, 0x6E, -1, -1, 0xB4, 0x6E, -1, -1, 0x00, 0x6E };
        EqAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000) + 0x40;

        if (EqAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find equipped status.", __FUNCTION__);
            continue;
        }

        stream << "IsEquipped: " << std::hex << EqAddr << std::endl;
        break;
    }

    offset += EqAddr - RotAddr;

    while (true) {
        sig = { 0x10, 0x3C, 0x71, 0xE4, -1, -1, 0x36, 0x84 };
        AnimAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000) + 0x08;

        if (AnimAddr < 30000) {
            sig = { 0x8A, 0xB9, 0x46, 0x6F, -1, 0x00, 0x00 };
            AnimAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000);

            if (AnimAddr < 30000) continue;
        }

        stream << "Animation: " << std::hex << AnimAddr << std::endl;
        break;
    }

    offset += AnimAddr - EqAddr;

    while (true) {
        sig = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, -1, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, -1, 0x3F, 0x80, 0x00, 0x00 };
        defAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false, 0x70000000) + 0x20;

        if (defAddr < 30000) {
            Logging::LoggerService::LogWarning("Failed to find defense.", __FUNCTION__);
            continue;
        }

        stream << "Def: " << std::hex << defAddr << std::endl;
        break;
    }

    offset += EqAddr - defAddr;

    uint64_t timeAddr;

    int attempt = 0;

    while (true) {
        std::vector<int> sig0 = { 0x10, 0x29, 0x7D, 0x40, -1, 0x2E, 0xE9, 0x50 };
        std::vector<int> sig1 = { 0x10, 0x29, 0x7D, 0x40, -1, 0x2E, 0xB2, 0xC0 };
        sig = attempt == 0 ? sig0 : sig1;
        timeAddr = Memory::PatternScan(sig, getBaseAddress(), 8, offset, false, false);

        if (timeAddr < 30000) {
            attempt = attempt == 0 ? 1 : 0;
            Logging::LoggerService::LogWarning("Failed to find time.", __FUNCTION__);
            continue;
        } else {
            timeAddr = timeAddr - 0x04;
        }

        stream << "Time: " << std::hex << timeAddr << std::endl;
        break;
    }

    Logging::LoggerService::LogDebug(stream.str());

    lastEquipment["W"] = "";
    lastEquipment["S"] = "";
    lastEquipment["B"] = "";
    lastEquipment["H"] = "";
    lastEquipment["U"] = "";
    lastEquipment["L"] = "";

    atkupAddr = RotAddr + 0xE0;

    result.push_back(dayAddr);
    result.push_back(timeAddr);

    Link_class::UpdateData();

    return result;
}

int Link_class::UpdateData() {
    int ReturnValue = 0;

    if (PosAddr + RotAddr + SpdAddr + AnimAddr > 30000) {
        Link_class::UpdatePosition();
        Link_class::UpdateRotation();
        Link_class::UpdateAnimation();
        Link_class::UpdateLocation();
        ReturnValue = Link_class::UpdateHealth();
        Link_class::UpdatePlayerEquipment();
        Link_class::UpdateAtkDef();
        Link_class::UpdateIsEquipped();
    }

    return ReturnValue;
}

void Link_class::UpdatePlayerEquipment() {
    Equipment.clear();

    for (int i = 0; i < 420; i++) {
        int isEquipped = Memory::read_bigEndian4Bytes(equippedAddr + (i * 8), __FUNCTION__);

        if (isEquipped == 1) {
            std::string equipmentName = "";

            for (int j = 0; j < 16; j++) {
                equipmentName += Memory::read_string(porchAddr + (i * 128) + (j * 8), 4, __FUNCTION__);
            }

            Equipment.push_back(equipmentName);
        }
    }
}

void Link_class::UpdateAtkDef() {
    atkup = Memory::read_bigEndianFloat(atkupAddr, __FUNCTION__);
    def = Memory::read_bigEndian4Bytes(defAddr, __FUNCTION__);
}

void Link_class::UpdateIsEquipped() {
    IsEquipped = Memory::read_bytes(EqAddr, 1, __FUNCTION__)[0] == 0x0 ? 1 : 0;
}

void Link_class::reduceHealth(int healthToReduce) {
    float timeSinceLastHit = float(GetTickCount() - lastHealthReduce) / 1000;

    if (timeSinceLastHit < 1) {
        return;
    }

    Memory::write_bigEndian4Bytes(HealthAddr, Health - healthToReduce, __FUNCTION__);
    lastHealthReduce = GetTickCount();
}

rapidjson::Document Link_class::readWeaponDamages() {
    char* home = getenv("HOME");
    std::string filepath = std::string(home) + "/.BOTWM/WeaponDamages.txt";

    std::ifstream file(filepath);
    std::stringstream buffer;
    buffer << file.rdbuf();

    rapidjson::Document doc;
    doc.Parse(buffer.str().c_str());

    return doc;
}

std::map<std::string, std::string> Link_class::GetPosition() {
    std::map<std::string, std::string> result;
    result["x"] = to_string_precision(Position[0], 4);
    result["y"] = to_string_precision(Position[1], 4);
    result["z"] = to_string_precision(Position[2], 4);

    return result;
}

std::string Link_class::GetRotation() {
    return to_string_precision(Rotation, 4);
}

std::string Link_class::GetAnimation() {
    return std::to_string(Animation);
}

std::map<std::string, std::string> Link_class::GetSpeed() {
    std::map<std::string, std::string> result;
    result["x"] = to_string_precision(Speed[0], 4);
    result["y"] = to_string_precision(Speed[1], 4);
    result["z"] = to_string_precision(Speed[2], 4);
    return result;
}

std::map<std::string, std::string> Link_class::GetLocation() {
    std::map<std::string, std::string> result;
    result["M"] = Map;
    result["S"] = Section;
    result["T"] = Town;

    return result;
}

std::string Link_class::GetHealth() {
    return std::to_string(Health);
}

std::map<std::string, std::string> Link_class::GetEquipment() {
    std::map<std::string, std::string> newEq;

    newEq["W"] = "";
    newEq["S"] = "";
    newEq["B"] = "";
    newEq["H"] = "Armor_Default_Head";
    newEq["U"] = "Armor_Default_Upper";
    newEq["L"] = "Armor_Default_Lower";

    for (int i = 0; i < Equipment.size(); i++) {
        if (Equipment[i].find("Weapon") != std::string::npos && Equipment[i].find("Shield") == std::string::npos && Equipment[i].find("Bow") == std::string::npos) {
            if (Equipment[i] != lastEquipment["W"] || 1 == 1) {
                newEq["W"] = Equipment[i];
                lastEquipment["W"] = Equipment[i];
            } else {
                newEq["W"] = ".";
            }
        } else if (Equipment[i].find("Weapon") != std::string::npos && Equipment[i].find("Shield") != std::string::npos) {
            if (Equipment[i] != lastEquipment["S"] || 1 == 1) {
                newEq["S"] = Equipment[i];
                lastEquipment["S"] = Equipment[i];
            } else {
                newEq["S"] = ".";
            }
        } else if (Equipment[i].find("Weapon") != std::string::npos && Equipment[i].find("Bow") != std::string::npos) {
            if (Equipment[i] != lastEquipment["B"] || 1 == 1) {
                newEq["B"] = Equipment[i];
                lastEquipment["B"] = Equipment[i];
            } else {
                newEq["B"] = ".";
            }
        } else if (Equipment[i].find("Head") != std::string::npos) {
            if (Equipment[i] != lastEquipment["H"] || 1 == 1) {
                newEq["H"] = Equipment[i];
                lastEquipment["H"] = Equipment[i];
            } else {
                newEq["H"] = ".";
            }
        } else if (Equipment[i].find("Upper") != std::string::npos) {
            if (Equipment[i] != lastEquipment["U"] || 1 == 1) {
                newEq["U"] = Equipment[i];
                lastEquipment["U"] = Equipment[i];
            } else {
                newEq["U"] = ".";
            }
        } else if (Equipment[i].find("Lower") != std::string::npos) {
            if (Equipment[i] != lastEquipment["L"] || 1 == 1) {
                newEq["L"] = Equipment[i];
                lastEquipment["L"] = Equipment[i];
            } else {
                newEq["L"] = ".";
            }
        }
    }

    return newEq;
}

std::string Link_class::GetAtkUp() {
    return std::to_string(atkup);
}

std::string Link_class::GetDef() {
    return std::to_string(def);
}

std::string Link_class::GetIsEquipped() {
    return std::to_string(IsEquipped);
}

std::string Link_class::GetDataString() {
    std::string dataString = to_string_precision(Position[0], 4) + "," + to_string_precision(Position[1], 4) + "," + to_string_precision(Position[2], 4) + ";" +
        to_string_precision(Rotation, 4) + ";" +
        std::to_string(Animation) + ";" +
        to_string_precision(Speed[0], 0) + "," + to_string_precision(Speed[1], 0) + "," + to_string_precision(Speed[2], 0) + ";" +
        Map + "," + Section + "," + Town;

    return dataString;
}

std::string Link_class::to_string_precision(float number, int precision) {
    std::stringstream stream;
    stream << std::fixed << std::setprecision(precision) << number;
    return stream.str();
}

void Link_class::Teleport(float newPosition[3]) {
    for (int i = 0; i < 3; i++)
        Memory::write_bigEndianFloat(PosAddr + (i * 0x4), newPosition[i], __FUNCTION__);
}

////////////////// Other player's class //////////////////

OtherPlayer_class::OtherPlayer_class(int PlayerNumber) {
    this->playerNumber = PlayerNumber;
}

uint64_t OtherPlayer_class::FindPosAddr() {
    uint64_t Addr = Memory::read_bigEndian4BytesOffset(this->BaseAddress + 0x3A0, __FUNCTION__);
    Addr = Memory::read_bigEndian4BytesOffset(Addr + 0x50, __FUNCTION__);
    Addr = Memory::read_bigEndian4BytesOffset(Addr + 0x4, __FUNCTION__);
    Addr = Memory::read_bigEndian4BytesOffset(Addr + 0x80, __FUNCTION__);
    Addr = Memory::read_bigEndian4BytesOffset(Addr, __FUNCTION__);
    Addr = Memory::read_bigEndian4BytesOffset(Addr + 0x5C, __FUNCTION__);
    Addr = Memory::read_bigEndian4BytesOffset(Addr + 0x18, __FUNCTION__) + Memory::getBaseAddress();

    return Addr + 0x50;
}

void OtherPlayer_class::ClearPointers() {
    this->BaseAddress = 0u;

    if (deleteAddr == 0) return;

    write_byte(deleteAddr, 0x00, __FUNCTION__);
}

void OtherPlayer_class::FindWeapons() {
    uint64_t TempWeaponAddr = Memory::read_bigEndian4BytesOffset(this->BaseAddress + 0x39C, __FUNCTION__);
    TempWeaponAddr = Memory::read_bigEndian4BytesOffset(TempWeaponAddr + 0x78, __FUNCTION__);
    TempWeaponAddr = Memory::read_bigEndian4BytesOffset(TempWeaponAddr + 0x244, __FUNCTION__);

    int i = 0;
    bool Found = false;

    while (!Found) {
        uint64_t temp = Memory::read_bigEndian4BytesOffset(TempWeaponAddr + (i * 0x4), __FUNCTION__);

        if (int(3518303375) == Memory::read_bigEndian4BytesOffset(temp + 0x10, __FUNCTION__)) {
            Found = true;
            TempWeaponAddr = Memory::read_bigEndian4BytesOffset(temp + 0x4, __FUNCTION__);
            RightHandWeapon = Memory::read_bigEndian4BytesOffset(TempWeaponAddr + 0x1C, __FUNCTION__) + Memory::getBaseAddress();
            LeftHandWeapon = Memory::read_bigEndian4BytesOffset(TempWeaponAddr + 0xA0, __FUNCTION__) + Memory::getBaseAddress();
        }

        i++;
    }
}

void OtherPlayer_class::FindArmor() {
    if (this->BaseAddress == 0)
        return;

    uint64_t TempArmorAddr = Memory::read_bigEndian4BytesOffset(this->BaseAddress + 0x39C, __FUNCTION__);
    TempArmorAddr = Memory::read_bigEndian4BytesOffset(TempArmorAddr + 0x6C, __FUNCTION__);
    TempArmorAddr = Memory::read_bigEndian4BytesOffset(TempArmorAddr + 0x4B0, __FUNCTION__);
    TempArmorAddr = Memory::read_bigEndian4BytesOffset(TempArmorAddr + 0x38, __FUNCTION__);

    for (int i = 0; i < 5; i++) {
        uint64_t temp = Memory::read_bigEndian4BytesOffset(TempArmorAddr, __FUNCTION__);
        ArmorAddresses[i] = Memory::read_bigEndian4BytesOffset(temp + 0xC, __FUNCTION__) + Memory::getBaseAddress();
        TempArmorAddr = Memory::read_bigEndian4BytesOffset(TempArmorAddr + 0x10, __FUNCTION__);
    }
}

void OtherPlayer_class::ClearWeapons() {
    std::string BASE_DEFAULT = "BaseHead";
    std::string RIGHT_DEFAULT = "RightHandWeaponLongName";
    std::string LEFT_DEFAULT = "LeftHandWeaponLongName";
    std::string CHEST_DEFAULT = "BaseChest";
    std::string UPPER_DEFAULT = "MP_Armor_Default_Upper";
    std::string LOWER_DEFAULT = "MP_Armor_Default_Lower";
    std::string HEAD_DEFAULT = "MP_Armor_Default_Head";

    Memory::write_string(RightHandWeapon, "Jugadorx" + RIGHT_DEFAULT, RIGHT_DEFAULT.size() + 8, __FUNCTION__);
    Memory::write_string(LeftHandWeapon, "Jugadorx" + LEFT_DEFAULT, LEFT_DEFAULT.size() + 8, __FUNCTION__);
}

void OtherPlayer_class::SetWeapons() {
    std::string RIGHT_DEFAULT = "RightHandWeaponLongName";
    std::string LEFT_DEFAULT = "LeftHandWeaponLongName";

    FindWeapons();

    uint64_t OldAddress = ArmorAddresses[0];

    FindArmor();

    WeaponChangeMutex.lock();

    Memory::write_string(RightHandWeapon, Equipment["W"], RIGHT_DEFAULT.size() + 8, __FUNCTION__);
    Memory::write_string(LeftHandWeapon, Equipment["S"], LEFT_DEFAULT.size() + 8, __FUNCTION__);

    if (OldAddress == ArmorAddresses[0])
        WeaponChanged = false;

    WeaponChangeMutex.unlock();
}

void OtherPlayer_class::SetArmor() {
    std::string BASE_DEFAULT = "BaseHead";
    std::string CHEST_DEFAULT = "BaseChest";
    std::string UPPER_DEFAULT = "MP_Armor_Default_Upper";
    std::string LOWER_DEFAULT = "MP_Armor_Default_Lower";
    std::string HEAD_DEFAULT = "MP_Armor_Default_Head";

    std::string BaseToWrite = BASE_DEFAULT;
    std::string UpperToWrite = Equipment["U"] == "" ? UPPER_DEFAULT : "MP_" + Equipment["U"];
    std::string LowerToWrite = Equipment["L"] == "" ? LOWER_DEFAULT : "MP_" + Equipment["L"];
    std::string HeadToWrite = Equipment["H"] == "" ? HEAD_DEFAULT : "MP_" + Equipment["H"];
    std::string ChestToWrite = CHEST_DEFAULT;

    if (UpperToWrite == "")
        UpperToWrite = UPPER_DEFAULT;

    if (UpperToWrite != "" && UpperToWrite != UPPER_DEFAULT)
        ChestToWrite = "EmptyModel";

    std::vector<std::string> HeadSwapArmors = { "180", "160", "171" };

    if (HeadToWrite == "MP_Armor_180_Head" || HeadToWrite == "MP_Armor_160_Head" || HeadToWrite == "MP_Armor_171_Head")
        BaseToWrite = "EmptyModel";

    if (ArmorAddresses[0] != 0) {
        try {
            Memory::write_string(ArmorAddresses[0], BaseToWrite, BASE_DEFAULT.size() + 2, __FUNCTION__);
            Memory::write_string(ArmorAddresses[1], ChestToWrite, CHEST_DEFAULT.size() + 2, __FUNCTION__);
            Memory::write_string(ArmorAddresses[2], HeadToWrite, HEAD_DEFAULT.size() + 2, __FUNCTION__);
            Memory::write_string(ArmorAddresses[3], LowerToWrite, LOWER_DEFAULT.size() + 2, __FUNCTION__);
            Memory::write_string(ArmorAddresses[4], UpperToWrite, UPPER_DEFAULT.size() + 2, __FUNCTION__);
        } catch (...) {
            std::cout << "Trying to set armor again" << std::endl;
        }
    }
}

int OtherPlayer_class::spawnAction(std::vector<std::string> local, std::vector<std::string> server, bool shouldExist, bool isPrint) {
    int action = 0;

    bool exists = Memory::read_bytes(existsAddr, 1, __FUNCTION__)[0] == 0x01 ? true : false;

    if (exists && !shouldExist) {
        action = -1;
    } else if (!exists && shouldExist) {
        action = 1;
    }

    if (isPrint) std::cout << exists << " " << shouldExist << " " << action << " | ";

    return action;
}

void OtherPlayer_class::setGlyphAddr(uint64_t GAddr) {
    this->GlyphAddr = GAddr;
}

uint64_t OtherPlayer_class::ScanData(uint64_t offset) {
    if (offset == 0) {
        offset = 0x40000000;
    }

    std::vector<std::vector<int>> sigs = {
        { 0x00, 0x00, 0x00, -1, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, 0x00, 0xD7, 0x31, 0x04, 0xA1 },
        { 0x00, 0x00, 0x00, -1, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, 0x00, 0xE6, 0xD9, 0x1E, 0x3C },
        { 0x00, 0x00, 0x00, -1, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, 0x00, 0x40, 0xAE, 0x15, 0x88 },
        { 0x00, 0x00, 0x00, -1, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, 0x00, 0x85, 0x09, 0x2B, 0x06 }
    };
    baseAnimationAddr = Memory::PatternScan(sigs[playerNumber - 1], getBaseAddress(), 8, offset) + 0xB;

    if (playerNumber == 1) {
        uint64_t startAddr = getBaseAddress();
        offset = baseAnimationAddr - startAddr - 0x500000;
    }

    lastAnimation = baseAnimationAddr + ((multiplier_animation[80 - 1] - 1) * 0x10);
    lastSchedule = baseAnimationAddr + (0x10 * 88) + (0x10 * (multiplier_schedule[11 - 1] - 1));

    write_byte(lastAnimation, 0x01, __FUNCTION__);
    write_byte(lastSchedule, 0x01, __FUNCTION__);

    sigs = {
    { 0xC4, 0x8C, -1, 0x8D, 0x43, 0x8C, 0x00, 0x00, 0x44, 0xEF, 0x5F, 0x3C, 0x71, 0x98, 0x24, 0xBA },
    { 0xC4, 0x8C, -1, 0x8D, 0x43, 0x8C, 0x00, 0x00, 0x44, 0xEF, 0x5F, 0x3C, 0x36, 0x38, 0x5E, 0x6A },
    { 0xC4, 0x8C, -1, 0x8D, 0x43, 0x8C, 0x00, 0x00, 0x44, 0xEF, 0x5F, 0x3C, 0x0B, 0x58, 0x77, 0xDA },
    { 0xC4, 0x8C, -1, 0x8D, 0x43, 0x8C, 0x00, 0x00, 0x44, 0xEF, 0x5F, 0x3C, 0xB9, 0x78, 0xAB, 0xCA }
    };
    PosAddr = Memory::PatternScan(sigs[playerNumber - 1], getBaseAddress(), 8, offset);

    for (int i = 0; i < 3; i++) {
        PosPoint[i] = reinterpret_cast<uint32_t*>(PosAddr + (i * 0x4));
    }

    sigs = {
    { 0x00, 0x00, -1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEC, 0x78, 0x65, 0x77 },
    { 0x00, 0x00, -1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAB, 0xD8, 0x1F, 0xA7 },
    { 0x00, 0x00, -1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x96, 0xB8, 0x36, 0x17 },
    { 0x00, 0x00, -1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24, 0x98, 0xEA, 0x07 }
    };
    RotAddr = Memory::PatternScan(sigs[playerNumber - 1], getBaseAddress(), 8, offset);

    sigs = {
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, 0x00, 0x73, 0x49, 0x25, 0x2B },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, 0x00, 0xEA, 0x40, 0x74, 0x91 },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, 0x00, 0x9D, 0x47, 0x44, 0x07 },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x09, 0x00, 0x00, 0x03, 0x23, 0xD1, 0xA4 }
    };
    deleteAddr = Memory::PatternScan(sigs[playerNumber - 1], getBaseAddress(), 8, offset) + 0xB;

    sigs = {
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x01, 0x00, -1, 0x52, 0x23, 0x7D, 0x49 },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x01, 0x00, -1, 0xDC, 0xAC, 0x7A, 0xAA },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x01, 0x00, -1, 0x10, 0x06, 0x7A, 0x34 },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x01, 0x00, -1, 0x1A, 0xC3, 0x73, 0x2D }
    };
    existsAddr = Memory::PatternScan(sigs[playerNumber - 1], getBaseAddress(), 8, offset) + 0xB;

    sigs = {
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x01, 0x00, 0x00, 0x8D, 0x05, 0xC2, 0x9B },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x01, 0x00, 0x00, 0xFA, 0x9B, 0x10, 0x6B },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x01, 0x00, 0x00, 0x61, 0x3E, 0x5C, 0x04 },
    { 0x00, 0x00, -1, 0x00, 0x10, 0x29, 0x84, 0x10, 0x00, 0x01, 0x00, 0x00, 0x15, 0xA6, 0xB5, 0x8B }
    };
    isEqAddr = Memory::PatternScan(sigs[playerNumber - 1], getBaseAddress(), 8, offset) + 0xB;

    sigs = {
    { 0xAA, 0x06, 0x8A, 0x95, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x86, 0x38 },
    { 0x2C, 0x92, 0xF8, 0x3B, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x86, 0x38 },
    { 0xE7, 0xCE, 0x2B, 0x9E, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x86, 0x38 },
    { 0xFA, 0xCB, 0x1B, 0x26, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x86, 0x38 }
    };
    NameAddr = Memory::PatternScan(sigs[playerNumber - 1], getBaseAddress(), 8, offset) - 0x20;

    sigs = {
    { 0xE9, 0xC2, 0xE3, 0xA4, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84, 0x10 },
    { 0x95, 0xA3, 0xC6, 0x7F, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84, 0x10 },
    { 0x08, 0xAC, 0x27, 0x09, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84, 0x10 },
    { 0x6D, 0x61, 0x8D, 0xC9, -1, 0x00, 0x00, 0x00, 0x10, 0x29, 0x84, 0x10 }
    };
    DisplayNameAddr = Memory::PatternScan(sigs[playerNumber - 1], getBaseAddress(), 8, offset) - 0x1;

    counter = 0;

    return offset;
}

void OtherPlayer_class::setGlyph(float Position[3]) {
    for (int i = 0; i < 3; i++) {
        if (Position[i] != 0) {
            Pos[i] = Position[i];
            Memory::write_bigEndianFloat(GlyphAddr + (i * 0x4), Pos[i], __FUNCTION__);
        }
    }
}

void OtherPlayer_class::setPosition(float Position[3]) {
    for (int i = 0; i < 3; i++) {
        if (Position[i] != 0) {
            Pos[i] = Position[i];
            int val_int;
            memcpy(&val_int, &Pos[i], 4);
            *PosPoint[i] = (uint32_t)Memory::swap_Endian(val_int);
        }
    }
}

void OtherPlayer_class::setRotation(float Rotation) {
    float correctedRotation = -Rotation + 90;
    float rotationRadians = (correctedRotation * 3.1416) / 180;
    float axis[] = { 0, 0, 0 };

    if (correctedRotation < 90) {
        axis[0] = 1;
    } else {
        axis[0] = -1;
    }

    axis[2] = tan(rotationRadians) * axis[0];

    for (int i = 0; i < 3; i++) {
        Memory::write_bigEndianFloat(RotAddr + (i * 0x4), axis[i], __FUNCTION__);
    }
}

void OtherPlayer_class::setAnimation(int Schedule, int Animation) {
    // Animation logic here
}

std::vector<uint64_t> OtherPlayer_class::findWeaponAddr() {
    std::vector<std::vector<int>> sigs = {
    { 0x4A, 0x75, 0x67, 0x61, 0x64, 0x6F, 0x72, 0x31, 0x52, 0x69, 0x67, 0x68, 0x74, 0x48, 0x61, 0x6E, 0x64, 0x57, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x4C, 0x6F, 0x6E, 0x67, 0x4E, 0x61, 0x6D, 0x65, 0x00, 0x4A, 0x75, 0x67, 0x61, 0x64, 0x6F, 0x72, 0x31, 0x4C, 0x65, 0x66, 0x74, 0x48, 0x61, -1, 0x64, 0x57, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x4C, 0x6F, 0x6E, 0x67, 0x4E, 0x61, 0x6D, 0x65 },
    { 0x4A, 0x75, 0x67, 0x61, 0x64, 0x6F, 0x72, 0x32, 0x52, 0x69, 0x67, 0x68, 0x74, 0x48, 0x61, 0x6E, 0x64, 0x57, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x4C, 0x6F, 0x6E, 0x67, 0x4E, 0x61, 0x6D, 0x65, 0x00, 0x4A, 0x75, 0x67, 0x61, 0x64, 0x6F, 0x72, 0x32, 0x4C, 0x65, 0x66, 0x74, 0x48, 0x61, -1, 0x64, 0x57, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x4C, 0x6F, 0x6E, 0x67, 0x4E, 0x61, 0x6D, 0x65 },
    { 0x4A, 0x75, 0x67, 0x61, 0x64, 0x6F, 0x72, 0x33, 0x52, 0x69, 0x67, 0x68, 0x74, 0x48, 0x61, 0x6E, 0x64, 0x57, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x4C, 0x6F, 0x6E, 0x67, 0x4E, 0x61, 0x6D, 0x65, 0x00, 0x4A, 0x75, 0x67, 0x61, 0x64, 0x6F, 0x72, 0x33, 0x4C, 0x65, 0x66, 0x74, 0x48, 0x61, -1, 0x64, 0x57, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x4C, 0x6F, 0x6E, 0x67, 0x4E, 0x61, 0x6D, 0x65 },
    { 0x4A, 0x75, 0x67, 0x61, 0x64, 0x6F, 0x72, 0x34, 0x52, 0x69, 0x67, 0x68, 0x74, 0x48, 0x61, 0x6E, 0x64, 0x57, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x4C, 0x6F, 0x6E, 0x67, 0x4E, 0x61, 0x6D, 0x65, 0x00, 0x4A, 0x75, 0x67, 0x61, 0x64, 0x6F, 0x72, 0x34, 0x4C, 0x65, 0x66, 0x74, 0x48, 0x61, -1, 0x64, 0x57, 0x65, 0x61, 0x70, 0x6F, 0x6E, 0x4C, 0x6F, 0x6E, 0x67, 0x4E, 0x61, 0x6D, 0x65 }
    };

    std::vector<uint64_t> result = Memory::PatternScanMultiple(sigs[playerNumber - 1], getBaseAddress(), 8, 0x12000000, false, 0x25000000);

    return result;
}

void OtherPlayer_class::deleteActor() {
    write_byte(deleteAddr, 0x01, __FUNCTION__);
}

bool OtherPlayer_class::checkExists() {
    Memory::write_byte(existsAddr, 0, __FUNCTION__);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    bool exists = Memory::read_bytes(existsAddr, 1, __FUNCTION__)[0] == 0x01 ? true : false;
    return exists;
}

void OtherPlayer_class::changeData(rapidjson::Value& playerData, float time, float coords[]) {
    float newPos[3] = { -1124.3922, 238, 1914.9768 };
    float newRot = 0;
    int newSchd = 0;
    int newAnim = 0;

    std::vector<std::string> axes = { "x", "y", "z" };

    OtherPlayer_class::setPosition(coords);

    if (glyphSync)
        setGlyph(coords);

    newRot = (float)playerData["R"].GetDouble();
    OtherPlayer_class::setRotation(newRot);

    newSchd = playerData["Schd"].GetInt();
    newAnim = playerData["Anim"].GetInt();

    OtherPlayer_class::setAnimation(newSchd, newAnim);
}

bool OtherPlayer_class::changeEq(rapidjson::Value& playerData) {
    std::string RIGHT_DEFAULT = "Jugador1RightHandWeaponLongName";
    std::string LEFT_DEFAULT = "Jugador1LeftHandWeaponLongName";

    bool isChanged = false;

    for (rapidjson::Value::ConstMemberIterator iter = playerData["E"].MemberBegin(); iter != playerData["E"].MemberEnd(); ++iter) {
        std::string EqVal = iter->value.GetString();
        std::string EqName = iter->name.GetString();

        if (Equipment[EqName] == EqVal) {
            continue;
        }

        if (EqVal != ".") {
            Equipment[EqName] = EqVal;

            if (EqName == "W" || EqName == "S") {
                std::vector<uint64_t> WeaponAddrs = findWeaponAddr();
                uint64_t offset = 0;

                for (int i = 0; i < WeaponAddrs.size(); i++) {
                    Memory::write_string(WeaponAddrs[i], Equipment["W"], RIGHT_DEFAULT.size(), __FUNCTION__);
                    Memory::write_string(WeaponAddrs[i] + offset, Equipment["S"], LEFT_DEFAULT.size(), __FUNCTION__);
                }

                isChanged = true;
            }
        }
    }

    return isChanged;
}

void OtherPlayer_class::setIsEquipped(int IsEquipped) {
    Memory::write_byte(isEqAddr, IsEquipped, __FUNCTION__);
}

void OtherPlayer_class::restartAnimations() {
    for (int s = 1; s < 16; s++) {
        if (lastSchedule != baseAnimationAddr + (0x10 * 88) + (0x10 * (multiplier_schedule[s - 1] - 1))) {
            Memory::write_byte(baseAnimationAddr + (0x10 * 88) + (0x10 * (multiplier_schedule[s - 1] - 1)), 0x0, __FUNCTION__);
        }
    }

    for (int a = 1; a < 89; a++) {
        if (lastAnimation != baseAnimationAddr + ((multiplier_animation[a - 1] - 1) * 0x10)) {
            Memory::write_byte(baseAnimationAddr + ((multiplier_animation[a - 1] - 1) * 0x10), 0x0, __FUNCTION__);
        }
    }
}

void OtherPlayer_class::changeName(std::string Name, int DisplayName) {
    std::string ActualName = "";

    if (Name.size() > 19) {
        ActualName = Name.substr(0, 19);
    } else {
        ActualName = Name;
    }

    ActualName += " HP: " + std::to_string(Health);

    Memory::write_string(NameAddr, ActualName, 32, __FUNCTION__);
    Memory::write_byte(DisplayNameAddr, DisplayName, __FUNCTION__);
}