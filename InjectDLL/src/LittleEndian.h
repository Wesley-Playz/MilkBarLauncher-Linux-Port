#pragma once

#include <Windows.h>
#include "Memory.h"

namespace DataTypes
{
	template <typename T>
	class LittleEndian
	{
	public:
		uint64_t address;
		T LastKnown;

		LittleEndian();
		LittleEndian(uint64_t Address, const char* caller);
		LittleEndian(uint64_t baseAddr, std::vector<int> offsets, int finalOffset, const char* caller);
		LittleEndian(std::vector<int> signature, uint64_t finalOffset, const char* caller, uint64_t scanOffset = -1);

		void setAddress(uint64_t Address, const char* caller, bool Validate = true);

		void set(T val, const char* caller);
		T get(const char* caller);

	private:
		DWORD* Pointer;
		bool AddressSet = false;
		const char* callerFunction = "";

		bool ValidateAddress(uint64_t Address, const char* caller);
	};

	template <typename T>
	LittleEndian<T>::LittleEndian()
	{

	}

	template <typename T>
	LittleEndian<T>::LittleEndian(uint64_t Address, const char* caller)
	{
		setAddress(Address, caller);
	}

	template <typename T>
	LittleEndian<T>::LittleEndian(uint64_t baseAddr, std::vector<int> offsets, int finalOffset, const char* caller)
	{
		uint64_t addr = Memory::ReadPointers(baseAddr, offsets, true) + finalOffset;

		setAddress(addr, caller);
	}

	template <typename T>
	LittleEndian<T>::LittleEndian(std::vector<int> signature, uint64_t finalOffset, const char* caller, uint64_t scanOffset)
	{
		bool saveOffset = false;
		if (scanOffset == (uint64_t)(-1))
		{
			scanOffset = Memory::ScanOffset;
			saveOffset = true;
		}

		uint64_t addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, scanOffset) + finalOffset;

		int retries = 0;
		while (addr - finalOffset < 30000)
		{
			Logging::LoggerService::LogDebug("Failed to find address", caller);
			Sleep(1000);

			if (retries == 15)
			{
				Logging::LoggerService::LogError("Could not find location address. Closing...");
				throw L"Failed to find mod address. Make sure that the mod is installed correctly in BCML and that BCML, Extended Memory and Multiplayer Utilities graphics packs are enabled.";
			}

			retries++;
			addr = Memory::PatternScan(signature, Memory::getBaseAddress(), 8, scanOffset) + finalOffset;
		}

		if (saveOffset)
			Memory::ScanOffset = addr - Memory::RegionStart - 0x50;

		setAddress(addr, caller);
	}

	template <typename T>
	void LittleEndian<T>::setAddress(uint64_t Address, const char* caller, bool Validate)
	{
		if (Validate)
			AddressSet = ValidateAddress(Address, caller);
		else
			AddressSet = true;

		if (AddressSet)
		{
			this->Pointer = (DWORD*)(Address);
			this->address = Address;
		}
	}

	template <typename T>
	void LittleEndian<T>::set(T val, const char* caller)
	{
		LastKnown = val;

		if (!AddressSet)
			return;

		memcpy(Pointer, &val, sizeof(T));
	}

	template <typename T>
	T LittleEndian<T>::get(const char* caller)
	{
		if (!AddressSet)
			return (T)0;

		memcpy(&LastKnown, Pointer, sizeof(T));
		return LastKnown;
	}

	template <typename T>
	bool LittleEndian<T>::ValidateAddress(uint64_t Address, const char* caller)
	{

		if (Address == 0)
			return false;

		MEMORY_BASIC_INFORMATION mbi{ 0 };
		DWORD protectflags = (PAGE_GUARD | PAGE_NOCACHE | PAGE_NOACCESS);

		if (VirtualQuery((LPCVOID)Address, &mbi, sizeof(mbi)))
		{
			if (mbi.Protect & protectflags || !(mbi.State & MEM_COMMIT)) {
				Logging::LoggerService::LogError("Failed to validate address.", caller);

				exit(1);
			}
		}

		return true;
	}

}