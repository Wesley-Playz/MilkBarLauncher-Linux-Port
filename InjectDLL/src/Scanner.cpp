#include "Memory.h"
#include <iostream>

uint64_t Memory::PatternScan(std::vector<int> signature, uint64_t baseAddr, int region, uint64_t regionOffset, bool Multiple, bool multipleRegions, uint64_t regionMaxOffset)
{

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    uint64_t startAddress = baseAddr;
    uint64_t endAddress = (uint64_t)(si.lpMaximumApplicationAddress);

    MEMORY_BASIC_INFORMATION Scanmbi{ 0 };
    DWORD protectflags = (PAGE_GUARD | PAGE_NOCACHE | PAGE_NOACCESS);

    int contador = 1;
    for (uint64_t i = startAddress; i < endAddress - signature.size();) {
        if (VirtualQuery((LPCVOID)i, &Scanmbi, sizeof(Scanmbi))) {
            if (Scanmbi.Protect & protectflags || !(Scanmbi.State & MEM_COMMIT)) {
                i += Scanmbi.RegionSize;

                contador++;
                continue; // if bad adress then dont read from it
            }

            if (region != 0 && contador < region)
            {
                i += Scanmbi.RegionSize;
                contador++;
                continue;
            }

            if (contador > region && region != 0 && !multipleRegions)
            {
                break;
            }

            //get last '?' position in pattern and use it to calculate the max shift value.
            //the last position in the pattern should never be a '?' -> we do not bother checking it
            uint64_t maxShift = signature.size() - 1;
            uint64_t maxIndex = signature.size() - 2;
            uint64_t wildCardIndex = 0;
            for (uint64_t i = 0; i < maxIndex + 1; i++) {
                if (signature.at(i) == -1) {
                    maxShift = maxIndex - i;
                    wildCardIndex = i;
                }
            }

            //initialize the shift table
            uint64_t shiftTable[256];
            for (uint64_t i = 0; i <= 255; i++) {
                shiftTable[i] = maxShift;
            }

            //fill shiftTable
            //forgot this in the video: Because max shift should always be '?' we only update the shift table for bytes to the right of the last '?'
            for (uint64_t i = wildCardIndex + 1; i < maxIndex; i++) {
                shiftTable[signature.at(i)] = maxIndex - i;
            }

            uint64_t startingAddress = 0;
            uint64_t endAddress = Scanmbi.RegionSize - signature.size();

            if (region != 0 && regionOffset != 0)
            {
                startingAddress = regionOffset;
            }

            if (region != 0 && regionMaxOffset != 0)
            {
                if (regionMaxOffset < endAddress)
                    endAddress = regionMaxOffset;
            }

            for (uint64_t currentIndex = startingAddress; currentIndex < endAddress;) {

                for (uint64_t sigIndex = maxIndex; sigIndex >= 0; sigIndex--) {
                    byte reading;

                    try 
                    {
                        reading = *(byte*)((uint64_t)Scanmbi.BaseAddress + currentIndex + sigIndex);
                    }
                    catch (const std::exception& ex)
                    {
                        std::stringstream stream;
                        stream << "Exception thrown: " << ex.what();
                        Logging::LoggerService::LogError(stream.str(), __FUNCTION__);
                        return NULL;
                    }
                    catch (...)
                    {
                        Logging::LoggerService::LogError("Catched unknown exception.", __FUNCTION__);
                        return NULL;
                    }

                    if (reading != signature.at(sigIndex) && signature.at(sigIndex) != -1) {
                        currentIndex += shiftTable[reading];
                        break;
                    }
                    else if (sigIndex == 0) {

                        if (signature.at(signature.size() - 1) != *(byte*)((uint64_t)Scanmbi.BaseAddress + currentIndex + signature.size() - 1))
                        {
                            currentIndex += 1;
                            break;
                        }

                        return (uint64_t)Scanmbi.BaseAddress + currentIndex;
                    }
                }
            }

            if (region != 0 && contador == region && !multipleRegions)
            {
                break;
            }

            i = (uint64_t)Scanmbi.BaseAddress + Scanmbi.RegionSize;
        }
    }
    return NULL;

}

std::vector<uint64_t> Memory::PatternScanMultiple(std::vector<int> signature, uint64_t baseAddr, int region, uint64_t regionOffset, bool multipleRegions, uint64_t regionMaxOffset, int expectedValues)
{
    std::vector<uint64_t> result;

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    uint64_t startAddress = baseAddr;
    uint64_t endAddress = (uint64_t)(si.lpMaximumApplicationAddress);

    MEMORY_BASIC_INFORMATION mbi{ 0 };
    DWORD protectflags = (PAGE_GUARD | PAGE_NOCACHE | PAGE_NOACCESS);

    int contador = 1;
    for (uint64_t i = startAddress; i < endAddress - signature.size();) {
        if (VirtualQuery((LPCVOID)i, &mbi, sizeof(mbi))) {
            if (mbi.Protect & protectflags || !(mbi.State & MEM_COMMIT)) {
                i += mbi.RegionSize;

                contador++;
                continue; // if bad adress then dont read from it
            }

            if (region != 0 && contador < region)
            {
                i += mbi.RegionSize;
                contador++;
                continue;
            }

            if (contador > region && region != 0 && !multipleRegions)
            {
                break;
            }


            //get last '?' position in pattern and use it to calculate the max shift value.
            //the last position in the pattern should never be a '?' -> we do not bother checking it
            uint64_t maxShift = signature.size() - 1;
            uint64_t maxIndex = signature.size() - 2;
            uint64_t wildCardIndex = 0;
            for (uint64_t i = 0; i < maxIndex + 1; i++) {
                if (signature.at(i) == -1) {
                    maxShift = maxIndex - i;
                    wildCardIndex = i;
                }
            }

            //initialize the shift table
            uint64_t shiftTable[256];
            for (uint64_t i = 0; i <= 255; i++) {
                shiftTable[i] = maxShift;
            }


            //fill shiftTable
            //forgot this in the video: Because max shift should always be '?' we only update the shift table for bytes to the right of the last '?'
            for (uint64_t i = wildCardIndex + 1; i < maxIndex; i++) {
                shiftTable[signature.at(i)] = maxIndex - i;
            }


            uint64_t startingAddress = 0;
            uint64_t endAddress = mbi.RegionSize - signature.size();

            if (region != 0 && regionOffset != 0)
            {
                startingAddress = regionOffset;
            }

            if (region != 0 && regionMaxOffset != 0)
            {
                if (regionMaxOffset < endAddress)
                    endAddress = regionMaxOffset;
            }

            for (uint64_t currentIndex = startingAddress; currentIndex < endAddress;) {

                for (uint64_t sigIndex = maxIndex; sigIndex >= 0; sigIndex--) {
                    byte reading = *(byte*)((uint64_t)mbi.BaseAddress + currentIndex + sigIndex);

                    if (reading != signature.at(sigIndex) && signature.at(sigIndex) != -1) {
                        currentIndex += shiftTable[reading];
                        break;
                    }
                    else if (sigIndex == 0) {

                        if (signature.at(signature.size() - 1) != *(byte*)((uint64_t)mbi.BaseAddress + currentIndex + signature.size() - 1))
                        {
                            currentIndex += 1;
                            break;
                        }

                        //return (uint64_t)mbi.BaseAddress + currentIndex;
                        result.push_back((uint64_t)mbi.BaseAddress + currentIndex);

                        if (result.size() == expectedValues) return result;

                        currentIndex++;
                        break;

                    }
                }
            }


            if (region != 0 && contador == region && !multipleRegions)
            {
                break;
            }

            i = (uint64_t)mbi.BaseAddress + mbi.RegionSize;
        }
    }
    return result;

}

uint64_t Memory::TryPatternScan(std::vector<int> signature, uint64_t baseAddr, int region, uint64_t regionOffset, bool Multiple, bool multipleRegions, uint64_t regionMaxOffset, int retries, std::string flagName)
{
    uint64_t result = 0;
    int retryCounter = 0;
    bool infinite = false;

    if (retries == 0)
        infinite = true;

    while (result < 30000 && (infinite || retryCounter < retries))
    {
        result = PatternScan(signature, baseAddr, region, regionOffset, Multiple, multipleRegions, regionMaxOffset);
        retryCounter++;

        if (result < 30000)
        {
            Logging::LoggerService::LogDebug("Could not complete pattern scan." + flagName != "" ? "Flag: " + flagName : "", __FUNCTION__);

            Sleep(2000);
        }
    }

    return result;
}

uint64_t Memory::findRegionBaseAddress(uint64_t baseAddr, int region)
{

    SYSTEM_INFO si;
    GetSystemInfo(&si);

    uint64_t startAddress = baseAddr;
    uint64_t endAddress = (uint64_t)(si.lpMaximumApplicationAddress);

    MEMORY_BASIC_INFORMATION mbi{ 0 };
    DWORD protectflags = (PAGE_GUARD | PAGE_NOCACHE | PAGE_NOACCESS);

    std::vector<uint64_t> result;

    uint64_t i = startAddress;

    int contador = 1;
    while(true){
        if (VirtualQuery((LPCVOID)i, &mbi, sizeof(mbi))) {
            if (mbi.Protect & protectflags || !(mbi.State & MEM_COMMIT)) {
                i += mbi.RegionSize;
                contador++;
                continue; // if bad adress then dont read from it
            }

            if (region != 0 && contador != region)
            {
                i += mbi.RegionSize;
                contador++;
                continue;
            }

            if (contador == region)
            {
                break;
            }
        }
    }

    return i;

}